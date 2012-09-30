/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "RenderingSystem.h"
#include "base/EntityManager.h"
#include <cmath>
#include <cassert>
#include <sstream>
#if defined(ANDROID) || defined(EMSCRIPTEN)
#else
#include <sys/inotify.h>
#include <GL/glew.h>
#include <unistd.h>
#endif

RenderingSystem::InternalTexture RenderingSystem::InternalTexture::Invalid;

INSTANCE_IMPL(RenderingSystem);

RenderingSystem::RenderingSystem() : ComponentSystemImpl<RenderingComponent>("Rendering") {
	nextValidRef = nextEffectRef = 1;
	opengles2 = false;
	currentWriteQueue = 0;
    cameraPosition = Vector2::Zero; 
#ifndef EMSCRIPTEN
	pthread_mutex_init(&mutexes, 0);
	pthread_cond_init(&cond, 0);
#endif

#if defined(ANDROID) || defined(EMSCRIPTEN)
    #else
    inotifyFd = inotify_init();
    #endif
    InternalTexture::Invalid.color = InternalTexture::Invalid.alpha = 0;
}

void RenderingSystem::setWindowSize(int width, int height, float sW, float sH) {
	windowW = width;
	windowH = height;
	screenW = sW;
	screenH = sH;
	GL_OPERATION(glViewport(0, 0, windowW, windowH))
}

RenderingSystem::Shader RenderingSystem::buildShader(const std::string& vsName, const std::string& fsName) {
	Shader out;
	LOGI("building shader ...");
	out.program = glCreateProgram();
	check_GL_errors("glCreateProgram");

	LOGI("Compiling shaders: %s/%s\n", vsName.c_str(), fsName.c_str());
	GLuint vs = compileShader(vsName, GL_VERTEX_SHADER);
	GLuint fs = compileShader(fsName, GL_FRAGMENT_SHADER);

	GL_OPERATION(glAttachShader(out.program, vs))
	GL_OPERATION(glAttachShader(out.program, fs))
	LOGI("Binding GLSL attribs\n");
	GL_OPERATION(glBindAttribLocation(out.program, ATTRIB_VERTEX, "aPosition"))
	GL_OPERATION(glBindAttribLocation(out.program, ATTRIB_UV, "aTexCoord"))
	GL_OPERATION(glBindAttribLocation(out.program, ATTRIB_POS_ROT, "aPosRot"))

	LOGI("Linking GLSL program\n");
	GL_OPERATION(glLinkProgram(out.program))

	GLint logLength;
	glGetProgramiv(out.program, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 1) {
		char *log = new char[logLength];
		glGetProgramInfoLog(out.program, logLength, &logLength, log);
		LOGW("GL shader program error: '%s'", log);
		delete[] log;
	}

	out.uniformMatrix = glGetUniformLocation(out.program, "uMvp");
	out.uniformColorSampler = glGetUniformLocation(out.program, "tex0");
	out.uniformAlphaSampler = glGetUniformLocation(out.program, "tex1");
	out.uniformColor= glGetUniformLocation(out.program, "vColor");
#ifdef USE_VBO
	out.uniformUVScaleOffset = glGetUniformLocation(out.program, "uvScaleOffset");
	out.uniformRotation = glGetUniformLocation(out.program, "uRotation");
	out.uniformScale = glGetUniformLocation(out.program, "uScale");
#endif

	glDeleteShader(vs);
	glDeleteShader(fs);

	return out;
}

void RenderingSystem::init() {
	const GLubyte* extensions = glGetString(GL_EXTENSIONS);
	pvrSupported = (strstr((const char*)extensions, "GL_IMG_texture_compression_pvrtc") != 0);
	
#ifdef GLES2_SUPPORT
		#ifdef USE_VBO
		defaultShader = buildShader("default_vbo.vs", "default.fs");
		defaultShaderNoAlpha = buildShader("default_vbo.vs", "default_no_alpha.fs");
		#else
		defaultShader = buildShader("default.vs", "default.fs");
		defaultShaderNoAlpha = buildShader("default.vs", "default_no_alpha.fs");
		#endif
		GL_OPERATION(glClearColor(0, 0, 0, 1.0))
#endif

	// create 1px white texture
	uint8_t data[] = {255, 255, 255, 255};
	GL_OPERATION(glGenTextures(1, &whiteTexture))
	GL_OPERATION(glBindTexture(GL_TEXTURE_2D, whiteTexture))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT))// CLAMP_TO_EDGE))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT))// CLAMP_TO_EDGE))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
	GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1,
                1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                data))

	// GL_OPERATION(glEnable(GL_BLEND))
	GL_OPERATION(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA))
	GL_OPERATION(glEnable(GL_DEPTH_TEST))
	GL_OPERATION(glDepthFunc(GL_GEQUAL))
#if defined(ANDROID) || defined(EMSCRIPTEN)
	GL_OPERATION(glClearDepthf(0.0))
 #else
    GL_OPERATION(glClearDepth(0.0))
 #endif
	// GL_OPERATION(glDepthRangef(0, 1))
	GL_OPERATION(glDepthMask(false))
	
#ifdef USE_VBO
	glGenBuffers(3, squareBuffers);
GLfloat sqArray[] = {
-0.5, -0.5, 0,0,0,
0.5, -0.5, 0,1,0,
-0.5, 0.5, 0,0,1,
0.5, 0.5, 0,1,1
};

GLfloat sqArrayRev[] = {
0.5, -0.5, 0,0,0,
0.5, 0.5, 0,1,0,
-0.5, -0.5, 0,0,1,
-0.5, 0.5, 0,1,1
};
unsigned short sqIndiceArray[] = {
	0,1,2,1,3,2
};
// Buffer d'informations de vertex
glBindBuffer(GL_ARRAY_BUFFER, squareBuffers[0]);
glBufferData(GL_ARRAY_BUFFER, sizeof(sqArray), sqArray, GL_STATIC_DRAW);

glBindBuffer(GL_ARRAY_BUFFER, squareBuffers[1]);
glBufferData(GL_ARRAY_BUFFER, sizeof(sqArrayRev), sqArrayRev, GL_STATIC_DRAW);

// Buffer d'indices
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squareBuffers[2]);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(sqIndiceArray), sqIndiceArray, GL_STATIC_DRAW);

#endif
}
static bool sortFrontToBack(const RenderingSystem::RenderCommand& r1, const RenderingSystem::RenderCommand& r2) {
	if (r1.z == r2.z) {
		if (r1.effectRef == r2.effectRef)
			return r1.texture < r2.texture;
		else
			return r1.effectRef < r2.effectRef;
	} else
		return r1.z > r2.z;
}

static bool sortBackToFront(const RenderingSystem::RenderCommand& r1, const RenderingSystem::RenderCommand& r2) {
	if (r1.z == r2.z) {
		if (r1.effectRef == r2.effectRef) {
			if (r1.texture == r2.texture) {
	            return r1.color < r2.color;
	        } else {
	            return r1.texture < r2.texture;
	        }
		} else {
			return r1.effectRef < r2.effectRef;
		}
	} else {
		return r1.z < r2.z;
    }
}

void RenderingSystem::DoUpdate(float dt __attribute__((unused))) {
	#ifndef EMSCRIPTEN
	pthread_mutex_lock(&mutexes);
	#endif
	std::vector<RenderCommand> opaqueCommands, semiOpaqueCommands;

    const float camLeft = (cameraPosition.X - screenW * 0.5);
    const float camRight = (cameraPosition.X + screenW * 0.5);
	/* render */
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;
		RenderingComponent* rc = (*it).second;

		if (rc->hide || rc->color.a <= 0) {
			continue;
		}

		const TransformationComponent* tc = TRANSFORM(a);

		RenderCommand c;
		c.z = tc->z;
		c.texture = rc->texture;
		c.effectRef = rc->effectRef;
		c.halfSize = tc->size * 0.5f;
		c.color = rc->color;
		c.position = tc->worldPosition;
		c.rotation = tc->worldRotation;
		c.uv[0] = Vector2::Zero;
		c.uv[1] = Vector2(1, 1);

        if (c.rotation == 0 && rc->opaqueType == RenderingComponent::FULL_OPAQUE) {
            // left culling !
            float cullLeftX = camLeft - (c.position.X - c.halfSize.X);
            if (cullLeftX > 0) {
                c.uv[0].X = cullLeftX / tc->size.X;
                c.uv[1].X -= cullLeftX / tc->size.X;
                c.halfSize.X -= 0.5 * cullLeftX;
                c.position.X += 0.5 * cullLeftX;
            }
            // right culling !
            float cullRightX = (c.position.X + c.halfSize.X) - camRight;
            if (cullRightX > 0) {
                c.uv[1].X -= cullRightX / tc->size.X;
                c.halfSize.X -= 0.5 * cullRightX;
                c.position.X -= 0.5 * cullRightX;
            }
        }

        if (c.texture != InvalidTextureRef) {
            TextureInfo& info = textures[c.texture];
            int atlasIdx = info.atlasIndex;
            if (atlasIdx >= 0 && atlas[atlasIdx].glref == InternalTexture::Invalid) {
                if (delayedAtlasIndexLoad.insert(atlasIdx).second) {
                    LOGW("Requested effective load of atlas '%s'", atlas[atlasIdx].name.c_str());
                }
            }
         }

         switch (rc->opaqueType) {
	         #ifdef USE_VBO
	         case RenderingComponent::OPAQUE_ABOVE:
	         case RenderingComponent::OPAQUE_UNDER:
	         #endif
         	case RenderingComponent::NON_OPAQUE:
	         	semiOpaqueCommands.push_back(c);
	         	break;
	         case RenderingComponent::FULL_OPAQUE:
	         	opaqueCommands.push_back(c);
	         	break;
	         #ifndef USE_VBO
	         case RenderingComponent::OPAQUE_ABOVE:
	         case RenderingComponent::OPAQUE_UNDER:
	         	RenderCommand cA = c, cU = c;
	         	cA.halfSize.Y = (tc->size * rc->opaqueSeparation).Y * 0.5;
	         	cU.halfSize.Y = (tc->size * (1 - rc->opaqueSeparation)).Y * 0.5;
	         	cA.position.Y = (tc->worldPosition + Vector2::Rotate(Vector2(0, cU.halfSize.Y), c.rotation)).Y;
	         	cU.position.Y = (tc->worldPosition - Vector2::Rotate(Vector2(0, cA.halfSize.Y), c.rotation)).Y;
	         	cA.uv[0].Y = cU.halfSize.Y / c.halfSize.Y; // offset;
	         	cA.uv[1].Y = cA.halfSize.Y / c.halfSize.Y; // scale;
	         	cU.uv[0].Y = 0;
	         	cU.uv[1].Y = cU.halfSize.Y / c.halfSize.Y; // scale;
	         	if (rc->opaqueType == RenderingComponent::OPAQUE_ABOVE) {
		         	semiOpaqueCommands.push_back(cU);
		         	opaqueCommands.push_back(cA);
	         	} else {
		         	semiOpaqueCommands.push_back(cA);
		         	opaqueCommands.push_back(cU);
	         	}
	         	break;
	         #endif
         }
	}

	std::sort(opaqueCommands.begin(), opaqueCommands.end(), sortFrontToBack);
	std::sort(semiOpaqueCommands.begin(), semiOpaqueCommands.end(), sortBackToFront);

	RenderQueue& outQueue = renderQueue[currentWriteQueue];

    RenderCommand dummy;
    dummy.texture = BeginFrameMarker;
    dummy.halfSize = cameraPosition;
    outQueue.commands.push(dummy);

	for(std::vector<RenderCommand>::iterator it=opaqueCommands.begin(); it!=opaqueCommands.end(); it++) {
		outQueue.commands.push(*it);
	}

	dummy.texture = DisableZWriteMarker;
	outQueue.commands.push(dummy);

	for(std::vector<RenderCommand>::iterator it=semiOpaqueCommands.begin(); it!=semiOpaqueCommands.end(); it++) {
		outQueue.commands.push(*it);
	}

	dummy.texture = EndFrameMarker;
	static unsigned int cccc = 0;
	dummy.rotateUV = cccc++;
	outQueue.commands.push(dummy);
	outQueue.frameToRender++;
	//LOGW("[%d] Added: %d + %d + 2 elt (%d frames) -> %d (%u)", currentWriteQueue, opaqueCommands.size(), semiOpaqueCommands.size(), outQueue.frameToRender, outQueue.commands.size(), dummy.rotateUV);
	#ifndef EMSCRIPTEN
pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutexes);
#endif
	//current = (current + 1) % 2;
#if defined(LINUX) && !defined(ANDROID) && !defined(EMSCRIPTEN)
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(inotifyFd, &fds);
    Vector2 s1, s2;
    struct timeval tv;
    memset(&tv, 0, sizeof(tv));
    if (select(inotifyFd + 1, &fds, NULL, NULL, &tv) > 0) {
        char buffer[8192];
        struct inotify_event *event;

        if (read(inotifyFd, buffer, sizeof(buffer)) > 0) {
            event = (struct inotify_event *) buffer;


            for (unsigned int i=0; i<notifyList.size(); i++) {
                if (event->wd == notifyList[i].wd) {
                    // reload asset
                    InternalTexture r;
                    loadTexture(notifyList[i].asset, s1, s2, r);
                    if (r != InternalTexture::Invalid) {
                        for (unsigned int j=0; j<atlas.size(); j++) {
                            if (notifyList[i].asset == atlas[j].name) {
                                for(std::map<TextureRef, TextureInfo>::iterator it=textures.begin(); it!=textures.end(); ++it) {
                                    if (it->second.glref == atlas[j].glref)
                                        it->second.glref = r;
                                }
                             atlas[j].glref = r;
                            }
                        }
                        if (assetTextures.find(notifyList[i].asset) != assetTextures.end()) {
	                        textures[assetTextures[notifyList[i].asset]].glref = r;
                        }
                    }
                    break;
                }
            }
        }
    }
#endif
}

bool RenderingSystem::isEntityVisible(Entity e) {
	return isVisible(TRANSFORM(e));
}

bool RenderingSystem::isVisible(TransformationComponent* tc) {
	const Vector2 halfSize = tc->size * 0.5;
	Vector2 pos = tc->worldPosition - cameraPosition;

	if (pos.X + halfSize.X < -screenW*0.5)
		return false;
	if (pos.X - halfSize.X > screenW*0.5)
		return false;
	if (pos.Y + halfSize.Y < -screenH * 0.5)
		return false;
	if (pos.Y - halfSize.Y > screenH * 0.5)
		return false;
	return true;
}

int RenderingSystem::saveInternalState(uint8_t** out) {
	int size = 0;
	for (std::map<std::string, TextureRef>::iterator it=assetTextures.begin(); it!=assetTextures.end(); ++it) {
		size += (*it).first.length() + 1;
		size += sizeof(TextureRef);
		size += sizeof(TextureInfo);
	}

	*out = new uint8_t[size];
	uint8_t* ptr = *out;
	for (std::map<std::string, TextureRef>::iterator it=assetTextures.begin(); it!=assetTextures.end(); ++it) {
		ptr = (uint8_t*) mempcpy(ptr, (*it).first.c_str(), (*it).first.length() + 1);
		ptr = (uint8_t*) mempcpy(ptr, &(*it).second, sizeof(TextureRef));
		ptr = (uint8_t*) mempcpy(ptr, &(textures[it->second]), sizeof(TextureInfo));
	}
	return size;
}

void RenderingSystem::restoreInternalState(const uint8_t* in, int size) {
	assetTextures.clear();
	textures.clear();
	nextValidRef = 1;
	nextEffectRef = 1;
	int idx = 0;
	while (idx < size) {
		char name[128];
		int i=0;
		do {
			name[i] = in[idx++];
		} while (name[i++] != '\0');
		TextureRef ref;
		memcpy(&ref, &in[idx], sizeof(TextureRef));
		idx += sizeof(TextureRef);
		TextureInfo info;
		memcpy(&info, &in[idx], sizeof(TextureInfo));
		idx += sizeof(TextureInfo);

		assetTextures[name] = ref;
		if (info.atlasIndex >= 0) {
			info.glref = atlas[info.atlasIndex].glref;
			textures[ref] = info;
		}
		nextValidRef = MathUtil::Max(nextValidRef, ref + 1);
	}
	for (std::map<std::string, EffectRef>::iterator it=nameToEffectRefs.begin();
		it != nameToEffectRefs.end();
		++it) {
		nextEffectRef = MathUtil::Max(nextEffectRef, it->second + 1);		
	}
}

EffectRef RenderingSystem::loadEffectFile(const std::string& assetName) {
	EffectRef result = DefaultEffectRef;
	std::string name(assetName);

	if (nameToEffectRefs.find(name) != nameToEffectRefs.end()) {
		result = nameToEffectRefs[name];
		return result;
	} else {
		result = nextEffectRef++;
		nameToEffectRefs[name] = result;
	}
	
	#ifdef USE_VBO
	ref2Effects[result] = buildShader("default_vbo.vs", assetName);
	#else
	ref2Effects[result] = buildShader("default.vs", assetName);
	#endif
	
	return result;
}

void RenderingSystem::reloadEffects() {
	for (std::map<std::string, EffectRef>::iterator it=nameToEffectRefs.begin();
		it != nameToEffectRefs.end();
		++it) {
		#ifdef USE_VBO
		ref2Effects[it->second] = buildShader("default_vbo.vs", it->first);
		#else
		ref2Effects[it->second] = buildShader("default.vs", it->first);
		#endif 		
	}
}
