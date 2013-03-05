#include "RenderingSystem.h"
#include "RenderingSystem_Private.h"
#include "base/EntityManager.h"
#include "TransformationSystem.h"
#include "CameraSystem.h"
#include <cmath>
#include <sstream>
#include "base/MathUtil.h"
#include "util/IntersectionUtil.h"
#include "opengl/OpenGLTextureCreator.h"
#if defined(ANDROID) || defined(EMSCRIPTEN)
#else
#include <sys/inotify.h>
#include <GL/glew.h>
#include <unistd.h>
#endif

#ifdef DEBUG
#include "base/Assert.h"
#endif
#ifdef INGAME_EDITORS
#include <AntTweakBar.h>
#endif

INSTANCE_IMPL(RenderingSystem);

RenderingSystem::RenderingSystem() : ComponentSystemImpl<RenderingComponent>("Rendering"), initDone(false) {
	nextEffectRef = 1;
    nextValidFBRef = 1;
	currentWriteQueue = 0;
    frameQueueWritable = true;
    newFrameReady = false;
#ifndef EMSCRIPTEN
    pthread_mutex_init(&mutexes[L_RENDER], 0);
    pthread_mutex_init(&mutexes[L_QUEUE], 0);
    pthread_mutex_init(&mutexes[L_TEXTURE], 0);
	pthread_cond_init(&cond[0], 0);
    pthread_cond_init(&cond[1], 0);
#endif

    RenderingComponent tc;
    componentSerializer.add(new Property<TextureRef>(OFFSET(texture, tc)));
    componentSerializer.add(new Property<EffectRef>(OFFSET(effectRef, tc)));
    componentSerializer.add(new Property<Color>(OFFSET(color, tc)));
    componentSerializer.add(new Property<bool>(OFFSET(hide, tc)));
    componentSerializer.add(new Property<bool>(OFFSET(mirrorH, tc)));
    componentSerializer.add(new Property<bool>(OFFSET(zPrePass, tc)));
    componentSerializer.add(new Property<bool>(OFFSET(fastCulling, tc)));
    componentSerializer.add(new Property<int>(OFFSET(opaqueType, tc)));
    componentSerializer.add(new Property<int>(OFFSET(cameraBitMask, tc)));

    InternalTexture::Invalid.color = InternalTexture::Invalid.alpha = 0;
    initDone = true;

    renderQueue = new RenderQueue[2];
}

RenderingSystem::~RenderingSystem() {
#ifndef EMSCRIPTEN
    pthread_mutex_destroy(&mutexes[L_RENDER]);
    pthread_mutex_destroy(&mutexes[L_QUEUE]);
    pthread_mutex_destroy(&mutexes[L_TEXTURE]);
    pthread_cond_destroy(&cond[0]);
    pthread_cond_destroy(&cond[1]);
#endif
    initDone = false;
    delete[] renderQueue;
}

void RenderingSystem::setWindowSize(int width, int height, float sW, float sH) {
	windowW = width;
	windowH = height;
	screenW = sW;
	screenH = sH;
	GL_OPERATION(glViewport(0, 0, windowW, windowH))
    #ifdef INGAME_EDITORS
    TwWindowSize(width, height);
    #endif
}

RenderingSystem::Shader RenderingSystem::buildShader(const std::string& vsName, const std::string& fsName) {
	Shader out;
	VLOG(1) << "building shader ...";
	out.program = glCreateProgram();
	check_GL_errors("glCreateProgram");

	VLOG(1) << "Compiling shaders: " << vsName << '/' << fsName;
	GLuint vs = compileShader(vsName, GL_VERTEX_SHADER);
	GLuint fs = compileShader(fsName, GL_FRAGMENT_SHADER);

	GL_OPERATION(glAttachShader(out.program, vs))
	GL_OPERATION(glAttachShader(out.program, fs))
	VLOG(2) << "Binding GLSL attribs";
	GL_OPERATION(glBindAttribLocation(out.program, ATTRIB_VERTEX, "aPosition"))
	GL_OPERATION(glBindAttribLocation(out.program, ATTRIB_UV, "aTexCoord"))
	GL_OPERATION(glBindAttribLocation(out.program, ATTRIB_POS_ROT, "aPosRot"))

	VLOG(2) << "Linking GLSL program";
	GL_OPERATION(glLinkProgram(out.program))

	GLint logLength;
	glGetProgramiv(out.program, GL_INFO_LOG_LENGTH, &logLength);
	if (logLength > 1) {
		char *log = new char[logLength];
		glGetProgramInfoLog(out.program, logLength, &logLength, log);
		LOG(FATAL) << "GL shader program error: '" << log << "'";
        
		delete[] log;
	}

	out.uniformMatrix = glGetUniformLocation(out.program, "uMvp");
	out.uniformColorSampler = glGetUniformLocation(out.program, "tex0");
	out.uniformAlphaSampler = glGetUniformLocation(out.program, "tex1");
	out.uniformColor= glGetUniformLocation(out.program, "vColor");
    out.uniformCamera = glGetUniformLocation(out.program, "uCamera");
#ifdef USE_VBO
	out.uniformUVScaleOffset = glGetUniformLocation(out.program, "uvScaleOffset");
	out.uniformRotation = glGetUniformLocation(out.program, "uRotation");
	out.uniformScaleZ = glGetUniformLocation(out.program, "uScaleZ");
#endif

	glDeleteShader(vs);
	glDeleteShader(fs);

	return out;
}

void RenderingSystem::init() {
    OpenGLTextureCreator::detectSupportedTextureFormat();

	#ifdef USE_VBO
	defaultShader = buildShader("default_vbo.vs", "default.fs");
	defaultShaderNoAlpha = buildShader("default_vbo.vs", "default_no_alpha.fs");
    defaultShaderEmpty = buildShader("default_vbo.vs", "empty.fs");
	#else
	defaultShader = buildShader("default.vs", "default.fs");
	defaultShaderNoAlpha = buildShader("default.vs", "default_no_alpha.fs");
    defaultShaderEmpty = buildShader("default.vs", "empty.fs");
	#endif
	GL_OPERATION(glClearColor(148.0/255, 148.0/255, 148.0/255, 1.0)) // temp setting for RR

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

    /*GL_OPERATION(glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST))
    GL_OPERATION(glEnable(GL_CULL_FACE))
    GL_OPERATION(glShadeModel(GL_SMOOTH))*/

	// GL_OPERATION(glEnable(GL_BLEND))
	GL_OPERATION(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA))
	GL_OPERATION(glEnable(GL_DEPTH_TEST))
	GL_OPERATION(glDepthFunc(GL_GREATER))
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
    #define EPSILON 0.0001
	if (MathUtil::Abs(r1.z - r2.z) <= EPSILON) {
		if (r1.effectRef == r2.effectRef) {
			if (r1.texture == r2.texture) {
                if (r1.flags != r2.flags)
                    return r1.flags > r2.flags;
                else
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

static inline void modifyQ(RenderingSystem::RenderCommand& r, const Vector2& offsetPos, const Vector2& size) {
    const Vector2 offset =  offsetPos * r.halfSize * 2 + size * r.halfSize * 2 * 0.5;
    r.position = r.position  + Vector2((r.mirrorH ? -1 : 1), 1) * Vector2::Rotate(- r.halfSize + offset, r.rotation);
    r.halfSize = size * r.halfSize;
}

static void modifyR(RenderingSystem::RenderCommand& r, const Vector2& offsetPos, const Vector2& size) {
    const Vector2 offset =  offsetPos * r.halfSize * 2 + size * r.halfSize * 2 * 0.5;
    r.position = r.position  + Vector2((r.mirrorH ? -1 : 1), 1) * Vector2::Rotate(- r.halfSize + offset, r.rotation);
    r.halfSize = size * r.halfSize;
    r.uv[0] = offsetPos;
    r.uv[1] = size;
}

static bool cull(float camLeft, float camRight, RenderingSystem::RenderCommand& c) {
    if (c.rotation == 0) {
        assert (c.halfSize.X != 0);
        const float invWidth = 1.0f / (2 * c.halfSize.X);
        // left culling !
        {
            float cullLeftX = camLeft - (c.position.X - c.halfSize.X);
            if (cullLeftX > 0) {
                if (cullLeftX >= 2 * c.halfSize.X)
                    return false;
                const float prop = cullLeftX * invWidth; // € [0, 1]
                if (!c.mirrorH) {
                    c.uv[0].X += prop * c.uv[1].X;
                }
                c.uv[1].X *= (1 - prop);
                c.halfSize.X *= (1 - prop);
                c.position.X += 0.5 * cullLeftX;
                return true;
            }
        }
        // right culling !
        {
            float cullRightX = (c.position.X + c.halfSize.X) - camRight;
            if (cullRightX > 0) {
                if (cullRightX >= 2 * c.halfSize.X)
                    return false;
                const float prop = cullRightX * invWidth; // € [0, 1]
                if (c.mirrorH) {
                    c.uv[0].X += prop * c.uv[1].X;
                }
                c.uv[1].X *= (1 - prop);
                c.halfSize.X *= (1 - prop);
                c.position.X -= 0.5 * cullRightX;
                return true;
            }
        }
    }
    return true;
}

void RenderingSystem::DoUpdate(float) {
    static unsigned int cccc = 0;
    RenderQueue& outQueue = renderQueue[currentWriteQueue];

    LOG_IF(WARNING, outQueue.count != 0) << "Non empty queue : " << outQueue.count << " (queue=" << currentWriteQueue << ')';

    // retrieve all cameras
    std::vector<Entity> cameras = theCameraSystem.RetrieveAllEntityWithComponent();
    // remove non active ones
    std::remove_if(cameras.begin(), cameras.end(), CameraSystem::isDisabled);
    // sort along z
    std::sort(cameras.begin(), cameras.end(), CameraSystem::sort);

    outQueue.count = 0;
    for (unsigned idx = 0; idx<cameras.size(); idx++) {
        const Entity camera = cameras[idx];
        const CameraComponent* camComp = CAMERA(camera);
        const TransformationComponent* camTrans = TRANSFORM(camera);

    	std::vector<RenderCommand> opaqueCommands, semiOpaqueCommands;

    	/* render */
        FOR_EACH_ENTITY_COMPONENT(Rendering, a, rc)
            bool ccc = rc->cameraBitMask & (0x1 << camComp->id);
    		if (rc->hide || rc->color.a <= 0 || !ccc ) {
    			continue;
    		}

    		const TransformationComponent* tc = TRANSFORM(a);
            if (!IntersectionUtil::rectangleRectangle(camTrans, tc)) {
                continue;
            }
            #ifdef DEBUG
            LOG_IF(WARNING, tc->worldZ <= 0 || tc->worldZ > 1) << "Entity '" << theEntityManager.entityName(a) << "' has invalid z value: " << tc->worldZ << ". Will not be drawn";
            #endif

    		RenderCommand c;
    		c.z = tc->worldZ;
    		c.texture = rc->texture;
    		c.effectRef = rc->effectRef;
    		c.halfSize = tc->size * 0.5f;
    		c.color = rc->color;
    		c.position = tc->worldPosition;
    		c.rotation = tc->worldRotation;
    		c.uv[0] = Vector2::Zero;
    		c.uv[1] = Vector2(1, 1);
            c.mirrorH = rc->mirrorH;
            c.fbo = rc->fbo;
            if (rc->zPrePass) {
                c.flags = (EnableZWriteBit | DisableBlendingBit | DisableColorWriteBit);
            } else if (rc->opaqueType == RenderingComponent::FULL_OPAQUE) {
                c.flags = (EnableZWriteBit | DisableBlendingBit | EnableColorWriteBit);
            } else {
                c.flags = (DisableZWriteBit | EnableBlendingBit | EnableColorWriteBit);
            }

            if (c.texture != InvalidTextureRef && !c.fbo) {
                const TextureInfo& info = textureLibrary.get(c.texture);
                int atlasIdx = info.atlasIndex;
                if (atlasIdx >= 0 && atlas[atlasIdx].glref == InternalTexture::Invalid) {
                    if (delayedAtlasIndexLoad.insert(atlasIdx).second) {
                        LOG(INFO) << "Requested effective load of atlas '" << atlas[atlasIdx].name << "'";
                    }
                }
                modifyQ(c, info.reduxStart, info.reduxSize);
             }

             switch (rc->opaqueType) {
             	case RenderingComponent::NON_OPAQUE:
    	         	semiOpaqueCommands.push_back(c);
    	         	break;
    	         case RenderingComponent::FULL_OPAQUE:
    	         	opaqueCommands.push_back(c);
    	         	break;
                 default:
                    LOG(WARNING) << "Entity will not be drawn";
                    break;
             }
        }


        outQueue.commands.resize(
            outQueue.count + opaqueCommands.size() + semiOpaqueCommands.size() + 1);
    	std::sort(opaqueCommands.begin(), opaqueCommands.end(), sortFrontToBack);
    	std::sort(semiOpaqueCommands.begin(), semiOpaqueCommands.end(), sortBackToFront);

        RenderCommand dummy;
        dummy.texture = BeginFrameMarker;
        packCameraAttributes(camTrans, camComp, dummy);
        outQueue.commands[outQueue.count] = dummy;
        outQueue.count++;
        std::copy(opaqueCommands.begin(), opaqueCommands.end(), outQueue.commands.begin() + outQueue.count);//&outQueue.commands[outQueue.count]);
        outQueue.count += opaqueCommands.size();
        // semiOpaqueCommands.front().flags = (DisableZWriteBit | EnableBlendingBit);
        std::copy(semiOpaqueCommands.begin(), semiOpaqueCommands.end(), outQueue.commands.begin() + outQueue.count); //&outQueue.commands[outQueue.count]);
        outQueue.count += semiOpaqueCommands.size();
    }

    outQueue.commands.reserve(outQueue.count + 1);
    RenderCommand dummy;
    dummy.texture = EndFrameMarker;
    dummy.rotateUV = cccc;
    outQueue.commands[outQueue.count++] = dummy;
    outQueue.count++;
    std::stringstream framename;
    framename << "create-frame-" << cccc;
    PROFILE("Render", framename.str(), InstantEvent);
    cccc++;
	//LOGW("[%d] Added: %d + %d + 2 elt (%d frames) -> %d (%u)", currentWriteQueue, opaqueCommands.size(), semiOpaqueCommands.size(), outQueue.frameToRender, outQueue.commands.size(), dummy.rotateUV);
    //LOGW("Wrote frame %d commands to queue %d", outQueue.count, currentWriteQueue);

#ifndef EMSCRIPTEN
    // Lock to not change queue while ther thread is reading it
    pthread_mutex_lock(&mutexes[L_RENDER]);
    // Lock for notifying queue change
    pthread_mutex_lock(&mutexes[L_QUEUE]);
#endif
    currentWriteQueue = (currentWriteQueue + 1) % 2;
    newFrameReady = true;
#ifndef EMSCRIPTEN
    pthread_cond_signal(&cond[C_FRAME_READY]);
    pthread_mutex_unlock(&mutexes[L_QUEUE]);
    pthread_mutex_unlock(&mutexes[L_RENDER]);
#endif
}

bool RenderingSystem::isEntityVisible(Entity e, int cameraIndex) const {
	return isVisible(TRANSFORM(e), cameraIndex);
}

bool RenderingSystem::isVisible(const TransformationComponent* tc, int cameraIndex) const {
    return true;
    #if 0
    if (cameraIndex < 0) {
        for (unsigned camIdx = 0; camIdx < cameras.size(); camIdx++) {
            if (isVisible(tc, camIdx)) {
                return true;
            }
        }
        return false;
    }
    const Camera& camera = cameras[cameraIndex];
	const Vector2 pos(tc->worldPosition - camera.worldPosition);
    const Vector2 camHalfSize(camera.worldSize * .5);

    const float biggestHalfEdge = MathUtil::Max(tc->size.X * 0.5, tc->size.Y * 0.5);
	if ((pos.X + biggestHalfEdge) < -camHalfSize.X) return false;
	if ((pos.X - biggestHalfEdge) > camHalfSize.X) return false;
	if ((pos.Y + biggestHalfEdge) < -camHalfSize.Y) return false;
	if ((pos.Y - biggestHalfEdge) > camHalfSize.Y) return false;
	return true;
    #endif
}

int RenderingSystem::saveInternalState(uint8_t** out) {
	int size = 0;
    LOG(WARNING) << "TODO";
    #if 0
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
    #endif
	return size;
}

void RenderingSystem::restoreInternalState(const uint8_t* in, int size) {
    LOG(WARNING) << "TODO";
    #if 0
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
    #endif
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

void RenderingSystem::setFrameQueueWritable(bool b) {
    if (frameQueueWritable == b || !initDone)
        return;
    LOG(INFO) << "Writable: " << b;
#ifndef EMSCRIPTEN
    pthread_mutex_lock(&mutexes[L_QUEUE]);
#endif
    frameQueueWritable = b;
#ifndef EMSCRIPTEN
    pthread_cond_signal(&cond[C_FRAME_READY]);
    pthread_mutex_unlock(&mutexes[L_QUEUE]);

    pthread_mutex_lock(&mutexes[L_RENDER]);
    pthread_cond_signal(&cond[C_RENDER_DONE]);
    pthread_mutex_unlock(&mutexes[L_RENDER]);
#endif
}

FramebufferRef RenderingSystem::createFramebuffer(const std::string& name, int width, int height) {
    Framebuffer fb;
    fb.width = width;
    fb.height = height;

    // create a texture object
    glGenTextures(1, &fb.texture);
    glBindTexture(GL_TEXTURE_2D, fb.texture);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // create a renderbuffer object to store depth info
    glGenRenderbuffers(1, &fb.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fb.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // create a framebuffer object
    glGenFramebuffers(1, &fb.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);

    // attach the texture to FBO color attachment point
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.texture, 0);
    // attach the renderbuffer to depth attachment point
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fb.rbo);

    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
        LOG(FATAL) << "FBO not complete: " << status;

    // switch back to window-system-provided framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    FramebufferRef result = nextValidFBRef++;
    nameToFramebuffer[name] = result;
    ref2Framebuffers[result] = fb;
    return result;
}

FramebufferRef RenderingSystem::getFramebuffer(const std::string& fbName) const {
    std::map<std::string, FramebufferRef>::const_iterator it = nameToFramebuffer.find(fbName);
    if (it == nameToFramebuffer.end())
        LOG(FATAL) << "Framebuffer '" << fbName << "' does not exist";
    return it->second;
}

void packCameraAttributes(
    const TransformationComponent* cameraTrans,
    const CameraComponent* cameraComp,
    RenderingSystem::RenderCommand& out) {
    out.uv[0] = cameraTrans->worldPosition;
    out.uv[1] = cameraTrans->size;
    out.z = cameraTrans->worldRotation;

    out.rotateUV = cameraComp->fb;
    out.color = cameraComp->clearColor;
}

void unpackCameraAttributes(
    const RenderingSystem::RenderCommand& in,
    TransformationComponent* camera,
    CameraComponent* ccc) {
    camera->worldPosition = in.uv[0];
    camera->size = in.uv[1];
    camera->worldRotation = in.z;

    ccc->fb = in.rotateUV;
    ccc->clearColor = in.color;
}

#ifdef INGAME_EDITORS
void RenderingSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    RenderingComponent* tc = Get(entity, false);
    if (!tc) return;
    TwAddVarRW(bar, "color", TW_TYPE_COLOR4F, &tc->color, "group=Rendering");
    TwAddVarRW(bar, "hide", TW_TYPE_BOOLCPP, &tc->hide, "group=Rendering");
    TwEnumVal opa[] = { {RenderingComponent::NON_OPAQUE, "NonOpaque"}, {RenderingComponent::FULL_OPAQUE, "Opaque"} };
    TwType op = TwDefineEnum("OpaqueType", opa, 2);
    TwAddVarRW(bar, "opaque", op, &tc->opaqueType, "group=Rendering");
    TwAddVarRW(bar, "effect", TW_TYPE_INT32, &tc->effectRef, "group=Rendering");
    TwAddVarRW(bar, "texture", TW_TYPE_INT32, &tc->texture, "group=Rendering");
}
#endif