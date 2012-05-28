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
}

void RenderingSystem::setWindowSize(int width, int height, float sW, float sH) {
	w = width;
	h = height;
	screenW = sW;
	screenH = sH;
	GL_OPERATION(glViewport(0, 0, w, h))
}

void RenderingSystem::init() {
#ifdef GLES2_SUPPORT
	if (opengles2) {
		defaultProgram = 0;
		LOGW("default prog: %u before", defaultProgram);
		defaultProgram = glCreateProgram();
		check_GL_errors("glCreateProgram");
		LOGW("default prog: %u after [isAProgram = %d]", defaultProgram, glIsProgram(defaultProgram));
		
		LOGI("Compiling shaders\n");
		GLuint vs = compileShader("default.vs", GL_VERTEX_SHADER);
		GLuint fs = compileShader("default.fs", GL_FRAGMENT_SHADER);

		GL_OPERATION(glAttachShader(defaultProgram, vs))
		GL_OPERATION(glAttachShader(defaultProgram, fs))
		LOGI("Binding GLSL attribs\n");
		GL_OPERATION(glBindAttribLocation(defaultProgram, ATTRIB_VERTEX, "aPosition"))
		GL_OPERATION(glBindAttribLocation(defaultProgram, ATTRIB_UV, "aTexCoord"))
		GL_OPERATION(glBindAttribLocation(defaultProgram, ATTRIB_COLOR, "aColor"))
		GL_OPERATION(glBindAttribLocation(defaultProgram, ATTRIB_POS_ROT, "aPosRot"))

		LOGI("Linking GLSL program\n");
		GL_OPERATION(glLinkProgram(defaultProgram))

		 GLint logLength;
	 	glGetProgramiv(defaultProgram, GL_INFO_LOG_LENGTH, &logLength);
		 if (logLength > 1)
		 {
		     char *log = new char[logLength];
	 		glGetProgramInfoLog(defaultProgram, logLength, &logLength, log);
		     LOGW("GL shader program error: '%s'", log);
		     delete[] log;
		 }

		uniformMatrix = glGetUniformLocation(defaultProgram, "uMvp");

		glDeleteShader(vs);
		glDeleteShader(fs);
		
		GL_OPERATION(glClearColor(0.2, 0.5, 0.1, 1.0))
	} else 
#endif	
			{
		GL_OPERATION(glEnable(GL_TEXTURE_2D))
		GL_OPERATION(glClearColor(0.2, 0.5, 0.1, 1.0))
		glAlphaFunc(GL_GREATER, 0.1);
		//glEnable(GL_ALPHA_TEST);
	
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
}
static bool sortFrontToBack(const RenderCommand& r1, const RenderCommand& r2) {
	if (r1.z == r2.z)
		return r1.texture < r2.texture;
	else
		return r1.z > r2.z;
}

static bool sortBackToFront(const RenderCommand& r1, const RenderCommand& r2) {
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
            if (atlasIdx >= 0 && atlas[atlasIdx].texture == 0) {
                if (delayedAtlasIndexLoad.insert(atlasIdx).second) {
                    LOGW("Requested effective load of atlas '%s'", atlas[atlasIdx].name.c_str());
                }
            }
         }
		
		if (c.color.a >= 1 && rc->drawGroup == RenderingComponent::FrontToBack)
			commands.push_back(c);
		else
			semiOpaqueCommands.push_back(c);
	}

	std::sort(commands.begin(), commands.end(), sortFrontToBack);
	std::sort(semiOpaqueCommands.begin(), semiOpaqueCommands.end(), sortBackToFront);
	
	for(std::vector<RenderCommand>::iterator it=commands.begin(); it!=commands.end(); it++) {
		renderQueue.push(*it);
	}
	RenderCommand dummy;
	dummy.texture = DisableZWriteMarker;
	renderQueue.push(dummy);
	for(std::vector<RenderCommand>::iterator it=semiOpaqueCommands.begin(); it!=semiOpaqueCommands.end(); it++) {
		renderQueue.push(*it);
	}
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
                    GLuint r =  loadTexture(notifyList[i].asset, s1, s2);
                    if (r > 0) {
                        for (unsigned int j=0; j<atlas.size(); j++) {
                            if (notifyList[i].asset == atlas[j].name) {
                                for(std::map<TextureRef, TextureInfo>::iterator it=textures.begin(); it!=textures.end(); ++it) {
                                    if (it->second.glref == atlas[j].texture)
                                        it->second.glref  = r;
                                }
                             atlas[j].texture = r;
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
			info.glref = atlas[info.atlasIndex].texture;
			textures[ref] = info;
		}
		nextValidRef = MathUtil::Max(nextValidRef, ref + 1);
	}
}
