/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



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
#include "opengl/GLState.h"
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
    componentSerializer.add(new Property<TextureRef>(Murmur::Hash("texture"), PropertyType::Texture, OFFSET(texture, tc), 0));
    componentSerializer.add(new Property<bool>(Murmur::Hash("show"), OFFSET(show, tc)));
    componentSerializer.add(new Property<uint8_t>(Murmur::Hash("flags"), OFFSET(flags, tc)));
    componentSerializer.add(new Property<int8_t>(Murmur::Hash("camera_bitmask"), OFFSET(cameraBitMask, tc)));
    componentSerializer.add(new Property<Color>(Murmur::Hash("color"), OFFSET(color, tc)));

    InternalTexture::Invalid.color = InternalTexture::Invalid.alpha = 0;
    initDone = true;

    renderQueue = new RenderQueue[2];

#if SAC_INGAME_EDITORS
    memset(&highLight, 0, sizeof(highLight));
    wireframe = false;
#endif
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
    defaultShaderNoTexture = effectLibrary.load(DEFAULT_NO_TEXTURE_FRAGMENT);

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

#if SAC_USE_VBO
    // 4 vertices per element (2 triangles with 2 shared vertices)
    GL_OPERATION(glBindBuffer(GL_ARRAY_BUFFER, glBuffers[1]))
    GL_OPERATION(glBindBuffer(GL_ARRAY_BUFFER, glBuffers[2]))
#endif

    GL_OPERATION(glActiveTexture(GL_TEXTURE0))

    glState.viewport.update(windowW, windowH);
    glState.clear.update(Color());
    glState.flags.current = OpaqueFlagSet;
    GL_OPERATION(glDepthMask(true))
    GL_OPERATION(glDisable(GL_BLEND))
    GL_OPERATION(glColorMask(true, true, true, true))
}

// [z][flags][effect][texture][color]
//   flags:      3 bits
//  effect:      8 bits
// atlasIndex:     8 bits
//   color:     32 bits
//   z:         4 bits
static uint64_t makeKeyOpaque(const RenderingSystem::RenderCommand& rc) {
    uint64_t key = 0;

    // end goal is to sort object by key
    // flags:   63...61
    key |= ((uint64_t)(rc.flags & 0x7)) << 61;
    // z:       60...53
    key |= (((uint64_t)rc.effectRef) & 0xFF) << 53;
    // texture: 52...45
    key |= ((uint64_t)rc.atlasIndex & 0xFF) << 45;
    // color:   44...12
    key |= ((uint64_t)rc.color.asInt()) << 12;
    uint64_t s = (((uint64_t)(rc.zi)) >> 20);
    // z:       11...00
    s = (~s) & 0x3;
    key |= s;

    return key;
}

// [z][flags][effect][texture][color]
//   z:         13 bits
//   flags:      3 bits
//  effect:      8 bits
// texture:      8 bits
//   color:     32 bits
static uint64_t makeKeyBlended(const RenderingSystem::RenderCommand& rc) {
    uint64_t key = 0;

    // z:       63...48
    uint64_t s = (((uint64_t)(rc.zi)) >> 15); // 14 bits, but we remove the leading sign bit
    key |= s << 48;
    // flags:   50...48
    // key |= ((uint64_t)(rc.flags & 0x7)) << 48;
    // effect:  47..40
    key |= (((uint64_t)rc.effectRef) & 0xFF) << 40;
    // texture: 39...32
    key |= ((uint64_t)(rc.atlasIndex & 0xFF)) << 32;
    // color:   31...00
    key |= rc.color.asInt();
    return key;
}

// This function is used to sort opaque sprites from front to back
// Note: the sort algorithm sort from min to max, so in this case, r1 < r2
// means r1.z > r2.z
static bool sortFrontToBack(const RenderingSystem::RenderCommand& r1, const RenderingSystem::RenderCommand& r2) {
    return r1.key < r2.key;
}

// This function is used to sort alpha-blended sprites from back to front.
static bool sortBackToFront(const RenderingSystem::RenderCommand& r1, const RenderingSystem::RenderCommand& r2) {
    return r1.key < r2.key;
}

static inline void modifyQ(RenderingSystem::RenderCommand& r, const glm::vec2& offsetPos, const glm::vec2& size) {
    const glm::vec2 offset =  offsetPos * r.halfSize * 2.0f + size * r.halfSize * 2.0f * 0.5f;
    r.position = r.position  + glm::vec2((r.rflags & RenderingFlags::MirrorHorizontal ? -1.0f : 1.0f), 1.0f) * glm::rotate(- r.halfSize + offset, r.rotation);
    r.halfSize = size * r.halfSize;
}

// offsetPos and size are in [0, 1] interval -> must be multiplied by object size when used
static void modifyR(RenderingSystem::RenderCommand& r, const glm::vec2& offsetPos, const glm::vec2& size) {
    const glm::vec2 fullSize(r.halfSize * 2.0f);
    const glm::vec2 newCenterFromBL = (offsetPos + size * 0.5f) * fullSize;
    r.position = r.position + glm::vec2((r.rflags & RenderingFlags::MirrorHorizontal ? -1.0f : 1.0f), 1.0f) * glm::rotate(newCenterFromBL - r.halfSize, r.rotation);
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
                if (!(c.rflags & RenderingFlags::MirrorHorizontal)) {
                    c.uv[0].x += prop * c.uv[1].x;
                }
                c.uv[1].x *= (1 - prop);
                c.halfSize.x *= (1 - prop);
                c.position.x += 0.5f * cullLeftX;
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
                if ((c.rflags & RenderingFlags::MirrorHorizontal)) {
                    c.uv[0].x += prop * c.uv[1].x;
                }
                c.uv[1].x *= (1 - prop);
                c.halfSize.x *= (1 - prop);
                c.position.x -= 0.5f * cullRightX;
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

#if SAC_DEBUG
    static unsigned int cccc = 0;
#endif
    RenderQueue& outQueue = renderQueue[currentWriteQueue];

#if SAC_DEBUG
    LOGV_IF(1, outQueue.count != 0, "Non empty queue : " << outQueue.count << " (queue=" << currentWriteQueue << ')');
#endif

    // retrieve all cameras
    auto cameras = theCameraSystem.RetrieveAllEntityWithComponent();
    // remove non active ones
    std::remove_if(cameras.begin(), cameras.end(), CameraSystem::isDisabled);
    // sort along order
#if SAC_USE_VECTOR_STORAGE
    std::sort(cameras.begin(), cameras.end(), CameraSystem::sort);
#else
    cameras.sort(CameraSystem::sort);
#endif

    // alloca here is dangerous
    RenderCommand* opaqueCommands = (RenderCommand*) malloc(entityCount() * sizeof(RenderCommand));
    RenderCommand* blendedCommands = (RenderCommand*) malloc(entityCount() * sizeof(RenderCommand));

    unsigned opaqueIndex = 0, blendedIndex = 0;
    outQueue.count = 0;
    for (auto camera: cameras) {
        const CameraComponent* camComp = CAMERA(camera);
        const TransformationComponent* camTrans = TRANSFORM(camera);

        const float cameraInvSize = 1.0f / (camTrans->size.x * camTrans->size.y);
        opaqueIndex = blendedIndex = 0;

        IntersectionUtil::AABB camAABB;
        IntersectionUtil::computeAABB(camTrans, camAABB);

        /* render */
        FOR_EACH_ENTITY_COMPONENT(Rendering, a, rc)
            bool ccc = rc->cameraBitMask & (0x1 << camComp->id);
            if (!rc->show || rc->color.a <= 0 || !ccc ) {
                continue;
            }

            const TransformationComponent* tc = TRANSFORM(a);

            {
                IntersectionUtil::AABB entityAABB;
                IntersectionUtil::computeAABB(tc, entityAABB, !(rc->flags & RenderingFlags::FastCulling));

                if (!IntersectionUtil::rectangleRectangleAABB(camAABB, entityAABB)) {
                    continue;
                }
            }

            LOGW_IF(tc->z <= 0 || tc->z > 1, "Entity '" << theEntityManager.entityName(a) <<
                "' has invalid z value: " << tc->z << ". Will not be drawn");

            RenderCommand c;
            c.z = tc->z;
            c.texture = c.atlasIndex = rc->texture;
            c.effectRef = rc->effectRef;
            c.halfSize = tc->size * 0.5f;
            c.color = rc->color;
            c.shapeType = (int)tc->shape;
            c.position = tc->position;
            c.rotation = tc->rotation;
            c.uv[0] = glm::vec2(0.0f);
            c.uv[1] = glm::vec2(1.0f);
            c.rflags = rc->flags;
#if SAC_DEBUG
            c.e = a;
#endif

            if (c.rflags & RenderingFlags::ZPrePass) {
                LOGT_EVERY_N(1000, "Hu, why are Z-pre-pass disabled?");
                continue;
//#if SAC_INGAME_EDITORS
//                if (highLight.zPrePass) {
//                    c.color.g = c.color.r = 0;
//                    c.color.a = 0.5;
//                    c.flags = DebugFlagSet;
//                    c.texture = InvalidTextureRef;
//                } else
//#endif
                c.flags = ZPrePassFlagSet;
            } else if (!(c.rflags & RenderingFlags::NonOpaque)) {
                c.flags = OpaqueFlagSet;
#if SAC_INGAME_EDITORS
                if (highLight.opaque)
                    c.color.g = 0;
#endif
            } else {
                c.flags = AlphaBlendedFlagSet;
#if SAC_INGAME_EDITORS
                if (highLight.nonOpaque) {
                    c.color.b = 0;
                }
#endif
            }

            if (c.texture != InvalidTextureRef && !(c.rflags & RenderingFlags::TextureIsFBO)) {
                const TextureInfo* info = textureLibrary.get(c.texture, false);
                if (info) {
                    int atlasIdx = c.atlasIndex = info->atlasIndex;
                    // If atlas texture is not loaded yet, load it
                    if (atlasIdx >= 0 && atlas[atlasIdx].ref == InvalidTextureRef) {
                        atlas[atlasIdx].ref = textureLibrary.load(atlas[atlasIdx].name.c_str());
                        LOGV(1, "Requested effective load of atlas '" << atlas[atlasIdx].name << "' -> ref=" << atlas[atlasIdx].ref);
                    }

                    // Only display the required area of the texture
                    modifyQ(c, info->reduxStart, info->reduxSize);

                    // Check if we can enable opaque-first optimisation. Conditions are:
                    // 1. blending-enabled sprite
                    // 2. alpha == 1
                    // 3. non empty opaque area
                    // 4. sprite is not a z prepass one
                    // 5. sprite cover at least 1.25% of the camera source area
                    if (c.rflags & RenderingFlags::NonOpaque &&
                        c.color.a >= 1 &&
                        info->opaqueSize != glm::vec2(0.0f) &&
                        !(c.rflags & RenderingFlags::ZPrePass) &&
                        ((c.halfSize.x * info->opaqueSize.x) * (c.halfSize.y * info->opaqueSize.y) * cameraInvSize) > 0.001) {
                        // add a smaller full-opaque block at the center
                        RenderCommand cCenter(c);
#if SAC_INGAME_EDITORS
                        cCenter.color = rc->color;
                        if (highLight.runtimeOpaque) {
                            cCenter.color.r = 0;
                        }
#endif
                        cCenter.flags = OpaqueFlagSet;

                        // Note: no need to take rotate info->rotate into account.
                        // (opaqueStart/Size attributes do not depend on this)
                        modifyR(cCenter, info->opaqueStart, info->opaqueSize);

                        if (cull(camTrans, cCenter)) {
                            cCenter.key = makeKeyOpaque(cCenter);
                            opaqueCommands[opaqueIndex++] = cCenter;
                        }

#if SAC_INGAME_EDITORS
                        if (highLight.nonOpaque) {
                            c.color.b = 0.f;
                            c.color.a *= 0.6f;
                        }
#endif
                        c.key = makeKeyBlended(c);
                        blendedCommands[blendedIndex++] = c;
                        continue;
                    }
                }
            }

             if (!(c.rflags & RenderingFlags::FastCulling) && tc->shape == Shape::Square) {
                if (!cull(camTrans, c)) {
                    continue;
                }
             }

            if (c.rflags & RenderingFlags::NonOpaque) {
#if SAC_INGAME_EDITORS
                if (highLight.nonOpaque) {
                    c.color.b = 0.f;
                    c.color.a *= 0.6f;
                }
#endif
                c.key = makeKeyBlended(c);
                blendedCommands[blendedIndex++] = c;
            } else {
                c.key = makeKeyOpaque(c);
                opaqueCommands[opaqueIndex++] = c;
            }
        END_FOR_EACH()

        unsigned cnt = outQueue.count + opaqueIndex + blendedIndex + 1;

        if (outQueue.commands.size() < cnt)
            outQueue.commands.resize(cnt);

        std::sort(opaqueCommands, opaqueCommands + opaqueIndex, sortFrontToBack);
        std::sort(blendedCommands, blendedCommands + blendedIndex, sortBackToFront);

        RenderCommand dummy;
        dummy.texture = BeginFrameMarker;
#if SAC_DEBUG
        dummy.e = 0;
#endif
        packCameraAttributes(camTrans, camComp, dummy);
        outQueue.commands[outQueue.count] = dummy;
        outQueue.count++;
        std::copy(opaqueCommands, opaqueCommands + opaqueIndex, outQueue.commands.begin() + outQueue.count);
        outQueue.count += opaqueIndex;
        std::copy(blendedCommands, blendedCommands + blendedIndex, outQueue.commands.begin() + outQueue.count);
        outQueue.count += blendedIndex;
    }

#if SAC_DEBUG
    float invSize = 400.0f / (theRenderingSystem.screenW * theRenderingSystem.screenH);
    for (int i=0; i<3; ++i)
        renderingStats[i].reset();
        std::for_each(outQueue.commands.begin(), outQueue.commands.begin() + outQueue.count,
            [this, invSize] (const RenderCommand& a) -> void {
            if (a.texture == BeginFrameMarker) return;
            if (a.flags & EnableZWriteBit) {
                if (a.flags & EnableColorWriteBit) {
                    renderingStats[0].count++;
                    renderingStats[0].area += a.halfSize.x * a.halfSize.y * invSize;
                } else {
                    renderingStats[2].count++;
                    renderingStats[2].area += a.halfSize.x * a.halfSize.y * invSize;
                }
            } else {
                renderingStats[1].count++;
                renderingStats[1].area += a.halfSize.x * a.halfSize.y * invSize;
            }
        }
    );
#endif

    free (opaqueCommands);
    free (blendedCommands);

    outQueue.commands.reserve(outQueue.count + 1);

    RenderCommand dummy;
    dummy.texture = EndFrameMarker;
#if SAC_DEBUG
    dummy.rotateUV = cccc;
#endif
    if (outQueue.commands.size() <= outQueue.count)
        outQueue.commands.push_back(dummy);
    else
        outQueue.commands[outQueue.count] = dummy;
    outQueue.count++;
    // outQueue.count++;
#if SAC_DEBUG
    std::stringstream framename;
    framename << "create-frame-" << cccc;
    PROFILE("Render", framename.str(), InstantEvent);
    cccc++;
#endif

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
    const auto& cameras = theCameraSystem.RetrieveAllEntityWithComponent();
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
    if (frameQueueWritable)
        return;
    setFrameQueueWritable(true);
}
void RenderingSystem::disableRendering() {
    setFrameQueueWritable(false);
}

void RenderingSystem::setFrameQueueWritable(bool b) {
#if ! SAC_EMSCRIPTEN
    mutexes[L_QUEUE].lock();
#endif
    LOGV(1, "Set rendering queue writable= " << b << " and flush queues");
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

    FramebufferRef result;
    if (nameToFramebuffer.find(name) == nameToFramebuffer.end()) {
        result = nextValidFBRef++;
        nameToFramebuffer[name] = result;
    } else {
        result = nameToFramebuffer.find(name)->second;
    }
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
    out.rflags = cameraComp->clear;

    out.flags = cameraComp->fb;
    out.color = cameraComp->clearColor;
}

void unpackCameraAttributes(
    const RenderingSystem::RenderCommand& in,
    TransformationComponent* camera,
    CameraComponent* ccc) {
    camera->position = in.uv[0];
    camera->size = in.uv[1];
    camera->rotation = in.z;

    ccc->fb = in.flags;
    ccc->clearColor = in.color;
    ccc->clear = in.rflags;
}
