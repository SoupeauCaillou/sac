#include "RenderingSystem.h"
#include "RenderingSystem_Private.h"

#include "base/EntityManager.h"

#include "TransformationSystem.h"
#include "CameraSystem.h"

#include <cmath>
#include <sstream>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "util/IntersectionUtil.h"
#include "opengl/OpenGLTextureCreator.h"

#if SAC_DEBUG
// #include <GL/glew.h>
#include <stdint.h>
#endif

#if SAC_INGAME_EDITORS
#include <AntTweakBar.h>
#endif

INSTANCE_IMPL(RenderingSystem);

RenderingSystem::RenderingSystem() : ComponentSystemImpl<RenderingComponent>("Rendering"), assetAPI(0), initDone(false) {
    nextValidFBRef = 1;
    currentWriteQueue = 0;
    frameQueueWritable = false;
    newFrameReady = false;
#if ! SAC_EMSCRIPTEN
    mutexes = new std::mutex[3];
    cond = new std::condition_variable[2];
#endif

    RenderingComponent tc;
    componentSerializer.add(new Property<TextureRef>("texture", PropertyType::Texture, OFFSET(texture, tc), 0));
    componentSerializer.add(new Property<EffectRef>("effectRef", OFFSET(effectRef, tc)));
    componentSerializer.add(new Property<Color>("color", OFFSET(color, tc)));
    componentSerializer.add(new Property<bool>("show", OFFSET(show, tc)));
    componentSerializer.add(new Property<bool>("fbo", OFFSET(fbo, tc)));
    componentSerializer.add(new Property<bool>("mirror_h", OFFSET(mirrorH, tc)));
    componentSerializer.add(new Property<bool>("z_pre_pass", OFFSET(zPrePass, tc)));
    componentSerializer.add(new Property<bool>("fast_culling", OFFSET(fastCulling, tc)));
    componentSerializer.add(new Property<int>("opaque_type", OFFSET(opaqueType, tc)));
    componentSerializer.add(new Property<int>("camera_bitmask", OFFSET(cameraBitMask, tc)));
    componentSerializer.add(new Property<int>("shape", OFFSET(shape, tc)));

    InternalTexture::Invalid.color = InternalTexture::Invalid.alpha = 0;
    initDone = true;

    renderQueue = new RenderQueue[2];

    // simple square
    #if 0
    shapes[0].points.push_back(glm::vec2(-0.5, -0.5));
    shapes[0].points.push_back(glm::vec2(0.5, -0.5));
    shapes[0].points.push_back(glm::vec2(-0.5, 0.5));
    shapes[0].points.push_back(glm::vec2(0.5, 0.5));
    shapes[0].supportUV = true;
    shapes[0].verticesCount = 6;
    unsigned short baseSquare[] = {0, 1, 2, 1, 3, 2};
    #endif
    for (unsigned i=0; i<Shape::Count; i++) {
        shapes[i] = Polygon::create((Shape::Enum)i);
    }
}

RenderingSystem::~RenderingSystem() {
#if ! SAC_EMSCRIPTEN
    delete[] mutexes;
    delete[] cond;
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
#if SAC_INGAME_EDITORS
    TwWindowSize(width, height);
#endif
}

void RenderingSystem::setWindowSize(const glm::vec2& windowSize, const glm::vec2& screenSize) {
    windowW = windowSize.x;
    windowH = windowSize.y;
    screenW = screenSize.x;
    screenH = screenSize.y;
    GL_OPERATION(glViewport(0, 0, windowW, windowH))
#if SAC_INGAME_EDITORS
    TwWindowSize(windowW, windowH);
#endif
}

void RenderingSystem::init() {
    LOGF_IF(!assetAPI, "AssetAPI must be set before init is called");
    OpenGLTextureCreator::detectSupportedTextureFormat();
    textureLibrary.init(assetAPI);
    effectLibrary.init(assetAPI);

	defaultShader = effectLibrary.load(DEFAULT_FRAGMENT);
	defaultShaderNoAlpha = effectLibrary.load(DEFAULT_NO_ALPHA_FRAGMENT);
    defaultShaderEmpty = effectLibrary.load(EMPTY_FRAGMENT);

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
	GL_OPERATION(glDepthFunc(GL_GREATER))
#if SAC_DESKTOP
    GL_OPERATION(glClearDepth(0.0))
#else
    GL_OPERATION(glClearDepthf(0.0))
#endif
	// GL_OPERATION(glDepthRangef(0, 1))
	GL_OPERATION(glDepthMask(false))

#if SAC_USE_VBO
	GL_OPERATION(glGenBuffers(3, glBuffers))
#else
    GL_OPERATION(glGenBuffers(1, glBuffers))
#endif

    // create a VBO for indices
    GL_OPERATION(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffers[0]))
    GL_OPERATION(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        sizeof(unsigned short) * MAX_BATCH_TRIANGLE_COUNT * 3, 0, GL_DYNAMIC_DRAW))
    GL_OPERATION(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0))

#if SAC_USE_VBO
    // 4 vertices per element (2 triangles with 2 shared vertices)
    GL_OPERATION(glBindBuffer(GL_ARRAY_BUFFER, glBuffers[1]))
    GL_OPERATION(glBufferData(GL_ARRAY_BUFFER,
        MAX_BATCH_TRIANGLE_COUNT * 3 * 3 * sizeof(float), 0, GL_DYNAMIC_DRAW))
    GL_OPERATION(glBindBuffer(GL_ARRAY_BUFFER, glBuffers[2]))
    GL_OPERATION(glBufferData(GL_ARRAY_BUFFER,
        MAX_BATCH_TRIANGLE_COUNT * 3 * 2 * sizeof(float), 0, GL_DYNAMIC_DRAW))
#endif
}


// The goal of this sort function is to group sprites to reduce OpenGL state changes.
static bool sortToMinizeStateChanges(const RenderingSystem::RenderCommand& r1, const RenderingSystem::RenderCommand& r2) {
    // 1st compare state (flags)
    if (r1.flags == r2.flags) {
        // 2nd: compare shader
        if (r1.effectRef == r2.effectRef) {
            // 3rd : compare texture
            if (r1.texture == r2.texture) {
                // 4th : compare color
                return r1.color < r2.color;
            } else {
                return r1.texture < r2.texture;
            }
        } else {
            return r1.effectRef < r2.effectRef;
        }
    } else {
        return r1.flags < r2.flags;
    }
}

// This function is used to sort opaque sprites from front to back
// Note: the sort algorithm sort from min to max, so in this case, r1 < r2
// means r1.z > r2.z
static bool sortFrontToBack(const RenderingSystem::RenderCommand& r1, const RenderingSystem::RenderCommand& r2) {
    static const double EPSILON = 0.0001;
    if (glm::abs(r1.z - r2.z) <= EPSILON) {
        return sortToMinizeStateChanges(r1, r2);
    } else {
        return r1.z > r2.z;
    }
}

// This function is used to sort alpha-blended sprites from back to front.
static bool sortBackToFront(const RenderingSystem::RenderCommand& r1, const RenderingSystem::RenderCommand& r2) {
    static const double EPSILON = 0.0001;
    if (glm::abs(r1.z - r2.z) <= EPSILON) {
        return sortToMinizeStateChanges(r1, r2);
    } else {
        return r1.z < r2.z;
    }

}

static inline void modifyQ(RenderingSystem::RenderCommand& r, const glm::vec2& offsetPos, const glm::vec2& size) {
    const glm::vec2 offset =  offsetPos * r.halfSize * 2.0f + size * r.halfSize * 2.0f * 0.5f;
    r.position = r.position  + glm::vec2((r.mirrorH ? -1.0f : 1.0f), 1.0f) * glm::rotate(- r.halfSize + offset, r.rotation);
    r.halfSize = size * r.halfSize;
}

// offsetPos and size are in [0, 1] interval -> must be multiplied by object size when used
static void modifyR(RenderingSystem::RenderCommand& r, const glm::vec2& offsetPos, const glm::vec2& size) {
    const glm::vec2 fullSize(r.halfSize * 2.0f);
    const glm::vec2 newCenterFromBL = (offsetPos + size * 0.5f) * fullSize;
    r.position = r.position + glm::vec2((r.mirrorH ? -1.0f : 1.0f), 1.0f) * glm::rotate(newCenterFromBL - r.halfSize, r.rotation);
    r.halfSize = size * r.halfSize;
    r.uv[0] = offsetPos;
    r.uv[1] = size;
}

static bool cull(const TransformationComponent* camera, RenderingSystem::RenderCommand& c) {
    if (c.rotation == 0 && c.halfSize.x > 0) {
        const float camLeft = camera->position.x - camera->size.x * .5f;
        const float camRight = camera->position.x + camera->size.x * .5f;

        const float invWidth = 1.0f / (2 * c.halfSize.x);
        // left culling !
        {
            float cullLeftX = camLeft - (c.position.x - c.halfSize.x);
            if (cullLeftX > 0) {
                if (cullLeftX >= 2 * c.halfSize.x)
                    return false;
                const float prop = cullLeftX * invWidth; // € [0, 1]
                if (!c.mirrorH) {
                    c.uv[0].x += prop * c.uv[1].x;
                }
                c.uv[1].x *= (1 - prop);
                c.halfSize.x *= (1 - prop);
                c.position.x += 0.5 * cullLeftX;
                return true;
            }
        }
        // right culling !
        {
            float cullRightX = (c.position.x + c.halfSize.x) - camRight;
            if (cullRightX > 0) {
                if (cullRightX >= 2 * c.halfSize.x)
                    return false;
                const float prop = cullRightX * invWidth; // € [0, 1]
                if (c.mirrorH) {
                    c.uv[0].x += prop * c.uv[1].x;
                }
                c.uv[1].x *= (1 - prop);
                c.halfSize.x *= (1 - prop);
                c.position.x -= 0.5 * cullRightX;
                return true;
            }
        }
    }
    return true;
}

#if SAC_LINUX && SAC_DESKTOP
void RenderingSystem::updateReload() {
    effectLibrary.updateReload();
    textureLibrary.updateReload();
}

void RenderingSystem::DoUpdate(float) {
    updateReload();

#else
void RenderingSystem::DoUpdate(float) {
#endif

    static unsigned int cccc = 0;
    RenderQueue& outQueue = renderQueue[currentWriteQueue];

    LOGW_IF(outQueue.count != 0, "Non empty queue : " << outQueue.count << " (queue=" << currentWriteQueue << ')');

    // retrieve all cameras
    std::vector<Entity> cameras = theCameraSystem.RetrieveAllEntityWithComponent();
    // remove non active ones
    std::remove_if(cameras.begin(), cameras.end(), CameraSystem::isDisabled);
    // sort along order
    std::sort(cameras.begin(), cameras.end(), CameraSystem::sort);

    outQueue.count = 0;
    for (unsigned idx = 0; idx<cameras.size(); idx++) {
        const Entity camera = cameras[idx];
        const CameraComponent* camComp = CAMERA(camera);
        const TransformationComponent* camTrans = TRANSFORM(camera);

        const float cameraInvSize = 1.0f / (camTrans->size.x * camTrans->size.y);
    	std::vector<RenderCommand> opaqueCommands, semiOpaqueCommands;

    	/* render */
        FOR_EACH_ENTITY_COMPONENT(Rendering, a, rc)
            bool ccc = rc->cameraBitMask & (0x1 << camComp->id);
    		if (!rc->show || rc->color.a <= 0 || !ccc ) {
    			continue;
    		}

    		const TransformationComponent* tc = TRANSFORM(a);
            if (!IntersectionUtil::rectangleRectangle(camTrans, tc)) {
                continue;
            }

            // LOGW_IF(tc->z <= 0 || tc->z > 1, "Entity '" << theEntityManager.entityName(a) << "' has invalid z value: " << tc->z << ". Will not be drawn")

    		RenderCommand c;
    		c.z = tc->z;
    		c.texture = rc->texture;
    		c.effectRef = rc->effectRef;
    		c.halfSize = tc->size * 0.5f;
    		c.color = rc->color;
            c.shapeType = (int)rc->shape;
            c.vertices = rc->dynamicVertices;
    		c.position = tc->position;
    		c.rotation = tc->rotation;
    		c.uv[0] = glm::vec2(0.0f);
    		c.uv[1] = glm::vec2(1.0f);
            c.mirrorH = rc->mirrorH;
            c.fbo = rc->fbo;
            if (rc->zPrePass) {
                // rc->opaqueType = RenderingComponent::FULL_OPAQUE;
                c.flags = (EnableZWriteBit | DisableBlendingBit | DisableColorWriteBit);
            } else if (rc->opaqueType == RenderingComponent::FULL_OPAQUE) {
                c.flags = (EnableZWriteBit | DisableBlendingBit | EnableColorWriteBit);
            } else {
                c.flags = (DisableZWriteBit | EnableBlendingBit | EnableColorWriteBit);
            }

            if (c.texture != InvalidTextureRef && !c.fbo) {
                const TextureInfo* info = textureLibrary.get(c.texture, false);
                if (info) {
                    int atlasIdx = info->atlasIndex;
                    // If atlas texture is not loaded yet, load it
                    if (atlasIdx >= 0 && atlas[atlasIdx].ref == InvalidTextureRef) {
                        atlas[atlasIdx].ref = textureLibrary.load(atlas[atlasIdx].name);
                        LOGI("Requested effective load of atlas '" << atlas[atlasIdx].name << "' -> ref=" << atlas[atlasIdx].ref);
                    }

                    // Only display the required area of the texture
                    modifyQ(c, info->reduxStart, info->reduxSize);

#if 1
                    // Check if we can enable opaque-first optimisation. Conditions are:
                    // 1. blending-enabled sprite
                    // 2. alpha == 1
                    // 3. non empty opaque area
                    // 4. sprite is not a z prepass one
                    // 5. sprite cover at least 1.25% of the camera source area
                    if (rc->opaqueType != RenderingComponent::FULL_OPAQUE &&
                        c.color.a >= 1 &&
                        info->opaqueSize != glm::vec2(0.0f) &&
                        !rc->zPrePass &&
                        (c.halfSize.x * c.halfSize.y * cameraInvSize) > 0.01) {
                        // add a smaller full-opaque block at the center
                        RenderCommand cCenter(c);
                        cCenter.flags = (EnableZWriteBit | DisableBlendingBit | EnableColorWriteBit);

                        // Note: no need to take rotate info->rotate into account.
                        // (opaqueStart/Size attributes do not depend on this)
                        modifyR(cCenter, info->opaqueStart, info->opaqueSize);

                        if (cull(camTrans, cCenter))
                            opaqueCommands.push_back(cCenter);
                        semiOpaqueCommands.push_back(c);
#if 0
                        const float leftBorder = info->opaqueStart.x, rightBorder = info->opaqueStart.x + info->opaqueSize.x;
                        const float bottomBorder = info->opaqueStart.y + info->opaqueSize.y;
                        if (leftBorder > 0) {
                            RenderCommand cLeft(c);
                            modifyR(cLeft, glm::vec2(0.0f), glm::vec2(leftBorder, 1.0f));
                            if (cull(camTrans, cLeft))
                                semiOpaqueCommands.push_back(cLeft);
                        }

                        if (rightBorder < 1) {
                            RenderCommand cRight(c);
                            modifyR(cRight, glm::vec2(rightBorder, 0.0f), glm::vec2(1 - rightBorder, 1.0f));
                            if (cull(camTrans, cRight))
                                semiOpaqueCommands.push_back(cRight);
                        }

                        RenderCommand cTop(c);
                        modifyR(cTop, glm::vec2(leftBorder, 0.0f), glm::vec2(rightBorder - leftBorder, info->opaqueStart.y));
                        if (cull(camTrans, cTop))
                            semiOpaqueCommands.push_back(cTop);

                        RenderCommand cBottom(c);
                        modifyR(cBottom, glm::vec2(leftBorder, bottomBorder), glm::vec2(rightBorder - leftBorder, 1 - bottomBorder));
                        if (cull(camTrans, cBottom))
                            semiOpaqueCommands.push_back(cBottom);
#endif
                        continue;
                    }
#endif
                }
            }

#if 1
             if (!rc->fastCulling && rc->shape == Shape::Square) {
                if (!cull(camTrans, c)) {
                    continue;
                }
             }
#endif

            switch (rc->opaqueType) {
             	case RenderingComponent::NON_OPAQUE:
    	         	semiOpaqueCommands.push_back(c);
    	         	break;
    	         case RenderingComponent::FULL_OPAQUE:
    	         	opaqueCommands.push_back(c);
    	         	break;
                 default:
                    LOGW("Entity will not be drawn");
                    break;
            }
        END_FOR_EACH()

        unsigned cnt = outQueue.count + opaqueCommands.size() + semiOpaqueCommands.size() + 1;

        if (outQueue.commands.size() < cnt)
            outQueue.commands.resize(cnt);

    	std::sort(opaqueCommands.begin(), opaqueCommands.end(), sortFrontToBack);
    	std::sort(semiOpaqueCommands.begin(), semiOpaqueCommands.end(), sortBackToFront);

        RenderCommand dummy;
        dummy.texture = BeginFrameMarker;
        packCameraAttributes(camTrans, camComp, dummy);
        outQueue.commands[outQueue.count] = dummy;
        outQueue.count++;
        std::copy(opaqueCommands.begin(), opaqueCommands.end(), outQueue.commands.begin() + outQueue.count);
        outQueue.count += opaqueCommands.size();
        std::copy(semiOpaqueCommands.begin(), semiOpaqueCommands.end(), outQueue.commands.begin() + outQueue.count);
        outQueue.count += semiOpaqueCommands.size();
    }

#if SAC_DEBUG
    float invSize = 400.0 / (theRenderingSystem.screenW * theRenderingSystem.screenH);
    for (int i=0; i<3; ++i)
        renderingStats[i].reset();
        std::for_each(outQueue.commands.begin(), outQueue.commands.begin() + outQueue.count,
            [this, invSize] (const RenderCommand& a) -> void {
            if (a.texture == BeginFrameMarker) return;
            if (a.flags & EnableZWriteBit) {
                if (a.flags & DisableColorWriteBit) {
                    renderingStats[2].count++;
                    renderingStats[2].area += a.halfSize.x * a.halfSize.y * invSize;
                } else {
                    renderingStats[0].count++;
                    renderingStats[0].area += a.halfSize.x * a.halfSize.y * invSize;
                }
            } else {
                renderingStats[1].count++;
                renderingStats[1].area += a.halfSize.x * a.halfSize.y * invSize;
            }
        }
    );
#endif

    outQueue.commands.reserve(outQueue.count + 1);

    RenderCommand dummy;
    dummy.texture = EndFrameMarker;
    dummy.rotateUV = cccc;
    if (outQueue.commands.size() <= outQueue.count)
        outQueue.commands.push_back(dummy);
    else
        outQueue.commands[outQueue.count] = dummy;
    outQueue.count++;
    // outQueue.count++;
    std::stringstream framename;
    framename << "create-frame-" << cccc;
    PROFILE("Render", framename.str(), InstantEvent);
    cccc++;
    //LOGW("[%d] Added: %d + %d + 2 elt (%d frames) -> %d (%u)", currentWriteQueue, opaqueCommands.size(), semiOpaqueCommands.size(), outQueue.frameToRender, outQueue.commands.size(), dummy.rotateUV);
    //LOGW("Wrote frame %d commands to queue %d", outQueue.count, currentWriteQueue);

#if ! SAC_EMSCRIPTEN
    // Lock to not change queue while ther thread is reading it
    mutexes[L_RENDER].lock();
    // Lock for notifying queue change
    mutexes[L_QUEUE].lock();
#endif
    currentWriteQueue = (currentWriteQueue + 1) % 2;
    newFrameReady = true;
#if ! SAC_EMSCRIPTEN
    cond[C_FRAME_READY].notify_all();
    mutexes[L_QUEUE].unlock();
    mutexes[L_RENDER].unlock();
#endif
}

bool RenderingSystem::isVisible(Entity e) const {
    return isVisible(TRANSFORM(e));
}

bool RenderingSystem::isVisible(const TransformationComponent* tc) const {
    std::vector<Entity> cameras = theCameraSystem.RetrieveAllEntityWithComponent();
    if (cameras.empty()) {
        return false;
    }
    TransformationComponent* camTrans = 0;
    for (auto& cam: cameras) {
        if (CAMERA(cam)->fb == DefaultFrameBufferRef) {
            camTrans = TRANSFORM(cam);
            break;
        }
    }
    if (!camTrans)
        return false;

    return IntersectionUtil::rectangleRectangle(camTrans, tc);

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
	const Vector2 pos(tc->position - camera.position);
    const Vector2 camHalfSize(camera.worldSize * .5);

    const float biggestHalfEdge = MathUtil::Max(tc->size.X * 0.5, tc->size.Y * 0.5);
	if ((pos.X + biggestHalfEdge) < -camHalfSize.X) return false;
	if ((pos.X - biggestHalfEdge) > camHalfSize.X) return false;
	if ((pos.Y + biggestHalfEdge) < -camHalfSize.Y) return false;
	if ((pos.Y - biggestHalfEdge) > camHalfSize.Y) return false;
	return true;
#endif
}

int RenderingSystem::saveInternalState(uint8_t** /*out*/) {
	int size = 0;
    LOGT("");
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

void RenderingSystem::restoreInternalState(const uint8_t* /*in*/, int /*size*/) {
    LOGT("");
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

void RenderingSystem::enableRendering() {
    if (frameQueueWritable || !initDone)
        return;
    setFrameQueueWritable(true);
}
void RenderingSystem::disableRendering() {
    if (!frameQueueWritable || !initDone)
        return;
    setFrameQueueWritable(false);
}

void RenderingSystem::setFrameQueueWritable(bool b) {
#if ! SAC_EMSCRIPTEN
    mutexes[L_QUEUE].lock();
#endif
    LOGI("Set rendering queue writable= " << b << " and flush queues");
    // Change writable state
    frameQueueWritable = b;
    // Flush queues
    renderQueue[0].count = 0;
    renderQueue[1].count = 0;
#if ! SAC_EMSCRIPTEN
    cond[C_FRAME_READY].notify_all();
    mutexes[L_QUEUE].unlock();

    mutexes[L_RENDER].lock();
    cond[C_RENDER_DONE].notify_all();
    mutexes[L_RENDER].unlock();
#endif
}

FramebufferRef RenderingSystem::createFramebuffer(const std::string& name, int width, int height) {
    Framebuffer fb;
    fb.width = width;
    fb.height = height;

    // create a texture object
    GL_OPERATION(glGenTextures(1, &fb.texture))
    GL_OPERATION(glBindTexture(GL_TEXTURE_2D, fb.texture))
    GL_OPERATION(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR))
    GL_OPERATION(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR))
    GL_OPERATION(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE))
    GL_OPERATION(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE))
#if SAC_DESKTOP
    GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE))
#endif
    GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0))
    GL_OPERATION(glBindTexture(GL_TEXTURE_2D, 0))

    // create a renderbuffer object to store depth info
    GL_OPERATION(glGenRenderbuffers(1, &fb.rbo))
    GL_OPERATION(glBindRenderbuffer(GL_RENDERBUFFER, fb.rbo))
#if ANDROID || SAC_EMSCRIPTEN
    GL_OPERATION(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height))
#else
    GL_OPERATION(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height))
#endif
    GL_OPERATION(glBindRenderbuffer(GL_RENDERBUFFER, 0))

    // create a framebuffer object
    GL_OPERATION(glGenFramebuffers(1, &fb.fbo))
    GL_OPERATION(glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo))

    // attach the texture to FBO color attachment point
    GL_OPERATION(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.texture, 0))
    // attach the renderbuffer to depth attachment point
    GL_OPERATION(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fb.rbo))

    // check FBO status
    GLenum status = GL_OPERATION(glCheckFramebufferStatus(GL_FRAMEBUFFER))
    if(status != GL_FRAMEBUFFER_COMPLETE)
        LOGE("FBO not complete: " << status);

    // switch back to window-system-provided framebuffer
    GL_OPERATION(glBindFramebuffer(GL_FRAMEBUFFER, 0))

    FramebufferRef result = nextValidFBRef++;
    nameToFramebuffer[name] = result;
    ref2Framebuffers[result] = fb;
    return result;
}

FramebufferRef RenderingSystem::getFramebuffer(const std::string& fbName) const {
    std::map<std::string, FramebufferRef>::const_iterator it = nameToFramebuffer.find(fbName);

    LOGF_IF(it == nameToFramebuffer.end(), "Framebuffer '" << fbName << "' does not exist");

    return it->second;
}

void packCameraAttributes(
    const TransformationComponent* cameraTrans,
    const CameraComponent* cameraComp,
    RenderingSystem::RenderCommand& out) {
    out.uv[0] = cameraTrans->position;
    out.uv[1] = cameraTrans->size;
    out.z = cameraTrans->rotation;

    out.rotateUV = cameraComp->fb;
    out.color = cameraComp->clearColor;
}

void unpackCameraAttributes(
    const RenderingSystem::RenderCommand& in,
    TransformationComponent* camera,
    CameraComponent* ccc) {
    camera->position = in.uv[0];
    camera->size = in.uv[1];
    camera->rotation = in.z;

    ccc->fb = in.rotateUV;
    ccc->clearColor = in.color;
}

void RenderingSystem::defineDynamicVertices(unsigned idx, const std::vector<glm::vec2>& v) {
#if !SAC_EMSCRIPTEN
    std::unique_lock<std::mutex> lock(mutexes[L_RENDER]);
#endif
    if (dynamicVertices.size() <= idx) {
        dynamicVertices.resize(idx + 1);
    }
    dynamicVertices[idx] = v;
#if !SAC_EMSCRIPTEN
    lock.unlock();
#endif
}
