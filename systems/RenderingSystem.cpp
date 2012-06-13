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
#ifndef ANDROID
#include <sys/inotify.h>
#include <GL/glew.h>
#include <unistd.h>
#else
#include <GLES/gl.h>
#endif

RenderingSystem::InternalTexture RenderingSystem::InternalTexture::Invalid;

INSTANCE_IMPL(RenderingSystem);

RenderingSystem::RenderingSystem() : ComponentSystemImpl<RenderingComponent>("Rendering") {
	nextValidRef = 1;
	opengles2 = false;
	current = 0;
	frameToRender = 0;
	pthread_mutex_init(&mutexes[0], 0);
	pthread_mutex_init(&mutexes[1], 0);
	pthread_cond_init(&cond, 0);
    #ifndef ANDROID
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

	glDeleteShader(vs);
	glDeleteShader(fs);
	
	return out;
}

void RenderingSystem::init() {
#ifdef GLES2_SUPPORT
	if (opengles2) {
		defaultShader = buildShader("default.vs", "default.fs");
		desaturateShader = buildShader("default.vs", "desaturate.fs");

		GL_OPERATION(glClearColor(0, 0, 0, 1.0))
	} else 
#endif	
			{
		GL_OPERATION(glEnable(GL_TEXTURE_2D))
		GL_OPERATION(glClearColor(0, 0, 0, 1.0))
		glDisable(GL_ALPHA_TEST);
	
		GL_OPERATION(glEnable(GL_TEXTURE_2D))
		GL_OPERATION(glMatrixMode(GL_PROJECTION))
		GL_OPERATION(glLoadIdentity())
     #ifdef ANDROID
        GL_OPERATION(glOrthof(-screenW*0.5, screenW*0.5, -screenH * 0.5, screenH * 0.5, 0, 1))
     #else
		GL_OPERATION(glOrtho(-screenW*0.5, screenW*0.5, -screenH * 0.5, screenH * 0.5, 0, 1))
     #endif
		GL_OPERATION(glMatrixMode(GL_MODELVIEW))
		GL_OPERATION(glLoadIdentity())
		GL_OPERATION(glEnableClientState(GL_VERTEX_ARRAY))
		GL_OPERATION(glEnableClientState(GL_COLOR_ARRAY))
		GL_OPERATION(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
	}

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
                
	GL_OPERATION(glEnable(GL_BLEND))
	GL_OPERATION(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA))
	// GL_OPERATION(glEnable(GL_DEPTH_TEST))
	GL_OPERATION(glDepthFunc(GL_GEQUAL))
 #ifdef ANDROID
	GL_OPERATION(glClearDepthf(0.0))
 #else
    GL_OPERATION(glClearDepth(0.0))
 #endif
	// GL_OPERATION(glDepthRangef(0, 1))
	GL_OPERATION(glDepthMask(false))
}
static bool sortFrontToBack(const RenderingSystem::RenderCommand& r1, const RenderingSystem::RenderCommand& r2) {
	if (r1.z == r2.z)
		return r1.texture < r2.texture;
	else
		return r1.z > r2.z;
}

static bool sortBackToFront(const RenderingSystem::RenderCommand& r1, const RenderingSystem::RenderCommand& r2) {
	if (r1.z == r2.z) {
		if (r1.texture == r2.texture) {
            return r1.color < r2.color;
        } else {
            return r1.texture < r2.texture;
        }
	} else {
		return r1.z < r2.z;
    }
}

void RenderingSystem::DoUpdate(float dt __attribute__((unused))) {
	pthread_mutex_lock(&mutexes[current]);
	
	std::vector<RenderCommand> commands;
	std::vector<RenderCommand> semiOpaqueCommands;

	/* render */
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;
		RenderingComponent* rc = (*it).second;
		
		if (rc->hide || rc->color.a <= 0) {
			continue;
		}

		const TransformationComponent* tc = TRANSFORM(a);

		RenderCommand c;
		c.texture = rc->texture;
		c.halfSize = tc->size * 0.5f;
		c.color = rc->color;
		c.position = tc->worldPosition;
		c.rotation = tc->worldRotation;
		c.z = tc->z;
		c.desaturate = rc->desaturate;

        if (c.texture != InvalidTextureRef) {
            TextureInfo& info = textures[c.texture];
            int atlasIdx = info.atlasIndex;
            if (atlasIdx >= 0 && atlas[atlasIdx].glref == InternalTexture::Invalid) {
                if (delayedAtlasIndexLoad.insert(atlasIdx).second) {
                    LOGW("Requested effective load of atlas '%s'", atlas[atlasIdx].name.c_str());
                }
            }
         }
		
		semiOpaqueCommands.push_back(c);
	}

	std::sort(semiOpaqueCommands.begin(), semiOpaqueCommands.end(), sortBackToFront);
	
	for(std::vector<RenderCommand>::iterator it=semiOpaqueCommands.begin(); it!=semiOpaqueCommands.end(); it++) {
		renderQueue.push(*it);
	}
	RenderCommand dummy;
	dummy.texture = EndFrameMarker;
	renderQueue.push(dummy);
	frameToRender++;
	// LOGW("Added: %d + %d + 1 elt (%d frames)", commands.size(), semiOpaqueCommands.size(), frameToRender);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutexes[current]);
	
	//current = (current + 1) % 2;
#ifndef ANDROID
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
	const Vector2& pos = tc->worldPosition;

	if (pos.X + halfSize.X < -screenW*0.5)
		return false;
	if (pos.X - halfSize.X > screenW*0.5)
		return false;
	if (pos.Y + halfSize.Y < -screenH * 0.5)
		return false;
	if (pos.Y - halfSize.Y > screenW * 0.5)
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
}
