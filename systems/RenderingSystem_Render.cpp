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
#include "CameraSystem.h"
#include "TransformationSystem.h"
#include <sstream>
#if SAC_INGAME_EDITORS
#include <AntTweakBar.h>
#include "util/LevelEditor.h"
#endif
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#if SAC_DEBUG
#include "RenderingSystem_Debug.h"
#endif

#define MAX_VERTEX_COUNT 512
#define MAX_INDICE_COUNT 1024

struct VertexData {
    glm::vec3 position;
    glm::vec2 uv;
};

static void computeVerticesScreenPos(const std::vector<glm::vec2>& points, const glm::vec2& position, const glm::vec2& hSize, float rotation, float z, VertexData* out);

bool firstCall;

RenderingSystem::ColorAlphaTextures RenderingSystem::chooseTextures(const InternalTexture& tex, const FramebufferRef& fbo, bool useFbo) {
    if (useFbo) {
        RenderingSystem::Framebuffer b = ref2Framebuffers[fbo];
        return std::make_pair(b.texture, b.texture);
    } else {
        return std::make_pair(tex.color, tex.alpha);
    }
}

static int drawBatchES2(
    const RenderingSystem::ColorAlphaTextures glref
    , const VertexData* vertices
    , const unsigned short* indices
    , int
#if SAC_USE_VBO
    batchVertexCount
#endif
    , int batchTriangleCount
    , unsigned indiceCount) {

    if (batchTriangleCount > 0) {
        GL_OPERATION(glBindTexture(GL_TEXTURE_2D, glref.first))

        if (firstCall) {
            // GL_OPERATION(glBindTexture(GL_TEXTURE_2D, 0))
        } else {
            GL_OPERATION(glActiveTexture(GL_TEXTURE1))
            GL_OPERATION(glBindTexture(GL_TEXTURE_2D, glref.second))
            GL_OPERATION(glActiveTexture(GL_TEXTURE0))
        }

#if SAC_USE_VBO
        // update vertex buffer
        GL_OPERATION(glBindBuffer(GL_ARRAY_BUFFER, theRenderingSystem.glBuffers[1]))
        // orphan previous storage
        GL_OPERATION(glBufferData(GL_ARRAY_BUFFER,
            MAX_VERTEX_COUNT * sizeof(VertexData), 0, GL_STREAM_DRAW))
        // update buffer
        GL_OPERATION(glBufferSubData(GL_ARRAY_BUFFER, 0,
            batchVertexCount * sizeof(VertexData), vertices))
        GL_OPERATION(glEnableVertexAttribArray(EffectLibrary::ATTRIB_VERTEX))

        GL_OPERATION(glVertexAttribPointer(EffectLibrary::ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), 0))
        GL_OPERATION(glEnableVertexAttribArray(EffectLibrary::ATTRIB_UV))
        GL_OPERATION(glVertexAttribPointer(EffectLibrary::ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), (void*)sizeof(glm::vec3)))

        GL_OPERATION(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theRenderingSystem.glBuffers[0]))
#else
        GL_OPERATION(glVertexAttribPointer(EffectLibrary::ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, sizeof(VertexData), &vertices[0].position))
        GL_OPERATION(glEnableVertexAttribArray(EffectLibrary::ATTRIB_VERTEX))
        GL_OPERATION(glVertexAttribPointer(EffectLibrary::ATTRIB_UV, 2, GL_FLOAT, GL_FALSE, sizeof(VertexData), &vertices[0].uv))
        GL_OPERATION(glEnableVertexAttribArray(EffectLibrary::ATTRIB_UV))
#endif

        // orphan
        GL_OPERATION(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            sizeof(unsigned short) * MAX_INDICE_COUNT, 0, GL_STREAM_DRAW))
        // update
        GL_OPERATION(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
            (indiceCount - 2) /*batchTriangleCount * 3*/ * sizeof(unsigned short), &indices[1]))

        GL_OPERATION(glDrawElements(GL_TRIANGLE_STRIP, indiceCount - 2/*batchTriangleCount * 3*/, GL_UNSIGNED_SHORT, 0))
    }

    #if SAC_OLD_HARDWARE
        //seems to solve artifacts on old graphic cards (nvidia 8600GT at least)
        glFinish();
    #endif
    return 0;
}

static inline void computeUV(RenderingSystem::RenderCommand& rc, const TextureInfo& info) {
    // Those 2 are used by RenderingSystem to display part of the texture, with different flags.
    // For instance: display a partial-but-opaque-version before the original alpha-blended one.
    // So, their default value are: offset=0,0 and size=1,1
    glm::vec2 uvModifierOffset(rc.uv[0]);
    glm::vec2 uvModifierSize(rc.uv[1]);
    const glm::vec2 uvSize (info.uv[1] - info.uv[0]);

     // If image is rotated in atlas, we need to adjust UVs
    if (info.rotateUV) {
        // #1: swap x/y start coordinates (ie: top-left point of the image)
        std::swap(uvModifierOffset.x, uvModifierOffset.y);
        // #2: swap x/y end coords (ie: bottom-right corner of the image)
        std::swap(uvModifierSize.x, uvModifierSize.y);
        // #3:
        uvModifierOffset.y = 1 - (uvModifierSize.y + uvModifierOffset.y);
        //uvModifierOffset.x = 1 - (uvModifierSize.x + uvModifierOffset.x);

    }

    // Compute UV to send to GPU
    {
        rc.uv[0] = info.uv[0] + glm::vec2(uvModifierOffset.x * uvSize.x, uvModifierOffset.y * uvSize.y);
        rc.uv[1] = rc.uv[0] + glm::vec2(uvModifierSize.x * uvSize.x, uvModifierSize.y * uvSize.y);
    }
    // Miror UV when doing horizontal miroring
    if (rc.rflags & RenderingFlags::MirrorHorizontal) {
        if (info.rotateUV)
            std::swap(rc.uv[0].y, rc.uv[1].y);
        else
            std::swap(rc.uv[0].x, rc.uv[1].x);
    }
    rc.rotateUV = info.rotateUV;
}

static inline void addRenderCommandToBatch(const RenderingSystem::RenderCommand& rc,
    const Polygon& polygon,
    VertexData* outVertices,
    unsigned short* outIndices,
    unsigned* verticesCount,
    unsigned* triangleCount,
    unsigned* indiceCount) {

    // vertices
    const std::vector<glm::vec2>& vert = polygon.vertices;

    // perform world -> screen position transformation
    computeVerticesScreenPos(vert, rc.position, rc.halfSize, rc.rotation, -rc.z, outVertices);

    // copy indices
    *outIndices++ = *verticesCount + polygon.indices[0];
    for(const auto& i: polygon.indices) {
        *outIndices++ = *verticesCount + i;
    }
    *outIndices++ = *verticesCount + polygon.indices.back();

    // copy uvs
    int mapping[][4] = {
        {0, 1, 3, 2},
        {1, 2, 0, 3}
    };
    outVertices[mapping[rc.rotateUV][0]].uv = glm::vec2(rc.uv[0].x, 1 - rc.uv[0].y);
    outVertices[mapping[rc.rotateUV][1]].uv = glm::vec2(rc.uv[1].x, 1 - rc.uv[0].y);
    outVertices[mapping[rc.rotateUV][2]].uv = glm::vec2(rc.uv[0].x, 1 - rc.uv[1].y);
    outVertices[mapping[rc.rotateUV][3]].uv = glm::vec2(rc.uv[1].x, 1 - rc.uv[1].y);

    *verticesCount += polygon.vertices.size();
    *triangleCount += polygon.indices.size() / 3;
    *indiceCount += 2 + polygon.indices.size();
}

EffectRef RenderingSystem::changeShaderProgram(EffectRef ref, bool _firstCall, bool useTexturing, const Color& color, const glm::mat4& mvp, bool colorEnabled) {
    const Shader& shader = effectRefToShader(ref, _firstCall, colorEnabled, useTexturing);
    // change active shader
    GL_OPERATION(glUseProgram(shader.program))
    // upload transform matrix (perspective + view)
    GL_OPERATION( glUniformMatrix4fv(shader.uniformMatrix, 1, GL_FALSE, glm::value_ptr(mvp)))
    // upload texture uniforms
    GL_OPERATION(glUniform1i(shader.uniformColorSampler, 0))
    GL_OPERATION(glUniform1i(shader.uniformAlphaSampler, 1))
    // upload color uniform
    GL_OPERATION(glUniform4fv(shader.uniformColor, 1, color.rgba))
    return ref;
}

void RenderingSystem::drawRenderCommands(RenderQueue& commands) {
    // Worst case scenario: 3 vertices per triangle (no shared vertices)
    static VertexData vertices[MAX_VERTEX_COUNT];
    static unsigned short indices[MAX_INDICE_COUNT];

    unsigned indiceCount = 0;
    // Rendering state
    struct {
        TransformationComponent worldPos;
        CameraComponent cameraAttr;
    } camera;
    InternalTexture boundTexture = InternalTexture::Invalid;
    FramebufferRef fboRef = DefaultFrameBufferRef;
    EffectRef currentEffect = InvalidTextureRef;
    Color currentColor(1,1,1,1);
    int currentFlags = glState.flags.current;
    bool useFbo = false;

    // Batch variable
    unsigned int batchVertexCount = 0, batchTriangleCount = 0;

    // matrices
    glm::mat4 camViewPerspMatrix;

    LOGV(2, "Begin frame rendering: " << commands.count);

    #if SAC_INGAME_EDITORS
    GL_OPERATION(glPolygonMode(GL_FRONT_AND_BACK, wireframe ? GL_LINE : GL_FILL))
    if (wireframe) {
        GL_OPERATION(glLineWidth(2))
        GL_OPERATION(glEnable(GL_LINE_SMOOTH))
    } else {
        GL_OPERATION(glDisable(GL_LINE_SMOOTH))
    }
    #endif


    // Setup initial GL state
    GL_OPERATION(glActiveTexture(GL_TEXTURE1))
    GL_OPERATION(glBindTexture(GL_TEXTURE_2D, 0))
    GL_OPERATION(glActiveTexture(GL_TEXTURE0))

    #if SAC_DEBUG
    batchSizes.clear();
    batchContent.clear();
    #endif

    #define TEX chooseTextures(boundTexture, fboRef, useFbo)

    // The idea here is to browse through the list of _ordered_ list of
    // render command to execute. We try to group (batch) them in single
    // GL commands. When a GL state change is required (new color, new
    // texture, etc), we flush (execute) the active batch, and start
    // building a new one.
    const unsigned count = commands.count;
    for (unsigned i=0; i<count; i++) {
        RenderCommand& rc = commands.commands[i];

        // HANDLE BEGIN/END FRAME MARKERS (new frame OR new camera)
        if (rc.texture == BeginFrameMarker) {
            #if SAC_DEBUG
            batchSizes.push_back(std::make_pair(BatchFlushReason::NewCamera, batchTriangleCount));
            #endif
            indiceCount = batchTriangleCount = batchVertexCount = drawBatchES2(TEX, vertices, indices, batchVertexCount, batchTriangleCount, indiceCount);

            PROFILE("Render", "begin-render-frame", InstantEvent);

            firstCall = true;
            unpackCameraAttributes(rc, &camera.worldPos, &camera.cameraAttr);
            LOGV(2, "   camera: pos=" << camera.worldPos.position.x << ',' << camera.worldPos.position.y
                << "size=" << camera.worldPos.size.x << ',' << camera.worldPos.size.y
                << " fb=" << camera.cameraAttr.fb);

            FramebufferRef fboRef = camera.cameraAttr.fb;
            if (fboRef == DefaultFrameBufferRef) {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glState.viewport.update(windowW, windowH);
            } else {
                const Framebuffer& fb = ref2Framebuffers[fboRef];
                glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
                glState.viewport.update(fb.width, fb.height);
            }

            // setup transformation matrix (based on camera attributes)
            camViewPerspMatrix =
                glm::ortho(-camera.worldPos.size.x * 0.5f, camera.worldPos.size.x * 0.5f,
                    -camera.worldPos.size.y * 0.5f, camera.worldPos.size.y * 0.5f,
                    0.0f, 1.0f) *
                glm::rotate( glm::mat4(1.0f),
                    -camera.worldPos.rotation, glm::vec3(0, 0, 1) ) *
                glm::translate( glm::mat4(1.0f),
                    glm::vec3(-camera.worldPos.position, 0.0f));

            // setup initial GL state
            currentEffect = changeShaderProgram(DefaultEffectRef, firstCall, true, currentColor, camViewPerspMatrix);

            glState.flags.update(OpaqueFlagSet);
            currentFlags = glState.flags.current;
            // GL_OPERATION(glDepthMask(true))
            /*GL_OPERATION(glDisable(GL_BLEND))
            GL_OPERATION(glColorMask(true, true, true, true))
            currentFlags = OpaqueFlagSet;*/
            if (camera.cameraAttr.clear) {
                glState.clear.update(camera.cameraAttr.clearColor);
                GL_OPERATION(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
            }
            continue;
        } else if (rc.texture == EndFrameMarker) {
            break;
        }

        // HANDLE RENDERING FLAGS (GL state switch)
        if (rc.flags != currentFlags) {
            #if SAC_DEBUG
            batchSizes.push_back(std::make_pair(BatchFlushInfo(BatchFlushReason::NewFlags, rc.flags), batchTriangleCount));
            #endif
            // flush batch before changing state
            indiceCount = batchTriangleCount = batchVertexCount = drawBatchES2(TEX, vertices, indices, batchVertexCount, batchTriangleCount, indiceCount);
            const bool useTexturing = (rc.texture != InvalidTextureRef);

            const int flagBitsChanged = glState.flags.update(rc.flags);

            // iff EnableBlendingBit changed
            if (flagBitsChanged & EnableBlendingBit ) {
                if (rc.flags & EnableBlendingBit) {
                    firstCall = false;
                    if (currentEffect == DefaultEffectRef) {
                        currentEffect = changeShaderProgram(DefaultEffectRef, firstCall, useTexturing, currentColor, camViewPerspMatrix);
                    }
                }
            }

            // iff EnableColorWriteBit changed
            if (flagBitsChanged & EnableColorWriteBit ) {
                if (currentEffect == DefaultEffectRef) {
                    currentEffect = changeShaderProgram(DefaultEffectRef, firstCall, useTexturing, currentColor, camViewPerspMatrix, rc.flags & EnableColorWriteBit);
                }
            }
            currentFlags = glState.flags.current;
        }
        // EFFECT HAS CHANGED ?
        if (rc.effectRef != currentEffect) {
            #if SAC_DEBUG
            batchSizes.push_back(std::make_pair(BatchFlushReason::NewEffect, batchTriangleCount));
            #endif
            // flush before changing effect
            indiceCount = batchTriangleCount = batchVertexCount = drawBatchES2(TEX, vertices, indices, batchVertexCount, batchTriangleCount, indiceCount);
            const bool useTexturing = (rc.texture != InvalidTextureRef);
            currentEffect = changeShaderProgram(rc.effectRef, firstCall, useTexturing, currentColor, camViewPerspMatrix, currentFlags & EnableColorWriteBit);
        }

        // SETUP TEXTURING
#if SAC_DEBUG
        const TextureRef rrr = rc.texture;
#endif
        const bool rcUseFbo = rc.rflags & RenderingFlags::TextureIsFBO;
        if (rc.texture != InvalidTextureRef) {
            if (!rcUseFbo) {
                const TextureInfo* info = textureLibrary.get(rc.texture, false);
                LOGF_IF(info == 0, "Invalid texture " << rc.texture << " : can not be found");
                if (info->atlasIndex >= 0) {
                    LOGF_IF((unsigned)info->atlasIndex >= atlas.size(), "Invalid atlas index: " << info->atlasIndex << " >= atlas count : " << atlas.size());
                    const TextureInfo* atlasInfo = textureLibrary.get(atlas[info->atlasIndex].ref, false);
                    LOGF_IF(!atlasInfo, "TextureInfo for atlas index: "
                        << info->atlasIndex << " not found (ref=" << atlas[info->atlasIndex].ref << ", name='" << atlas[info->atlasIndex].name << "')");
                    rc.glref = atlasInfo->glref;
                } else {
                    rc.glref = info->glref;
                }
                computeUV(rc, *info);
            } else {
                rc.uv[0] = glm::vec2(0, 1);
                rc.uv[1] = glm::vec2(1, 0);
                rc.rotateUV = 0;
            }
            if (rc.glref.color == 0)
                rc.glref.color = whiteTexture;
        } else {
            rc.glref = InternalTexture::Invalid;
            // hum
            if (firstCall) {
                rc.glref.color = whiteTexture;
                rc.glref.alpha = whiteTexture;
            }
            rc.uv[0] = glm::vec2(0.0f, 0.0f);
            rc.uv[1] = glm::vec2(1.0f, 1.0f);
            rc.rotateUV = 0;
        }

        // TEXTURE OR COLOR HAS CHANGED ?
        const bool condUseFbo = (useFbo != rcUseFbo);
        const bool condTexture = (!rcUseFbo && boundTexture != rc.glref && (currentFlags & EnableColorWriteBit));
        const bool condFbo = (rcUseFbo && fboRef != rc.framebuffer);
        const bool condColor = (currentColor != rc.color);
        if (condUseFbo | condTexture | condFbo | condColor) {
            #if SAC_DEBUG
            if (condUseFbo) {
                batchSizes.push_back(std::make_pair(BatchFlushReason::NewTarget, batchTriangleCount));
            } else if (condTexture) {
                batchSizes.push_back(std::make_pair(BatchFlushInfo(BatchFlushReason::NewTexture, rrr), batchTriangleCount));
            } else if (condColor) {
                batchSizes.push_back(std::make_pair(BatchFlushInfo(BatchFlushReason::NewColor, rc.color), batchTriangleCount));
            } else if (condColor) {
                batchSizes.push_back(std::make_pair(BatchFlushReason::NewFBO, batchTriangleCount));
            }
            #endif
            // flush before changing texture/color
            indiceCount = batchTriangleCount = batchVertexCount = drawBatchES2(TEX, vertices, indices, batchVertexCount, batchTriangleCount, indiceCount);
            if (rcUseFbo) {
                fboRef = rc.framebuffer;
                boundTexture = InternalTexture::Invalid;
            } else {
                fboRef = DefaultFrameBufferRef;
                boundTexture = rc.glref;
#if SAC_INGAME_EDITORS
                if (highLight.nonOpaque)
                    boundTexture.alpha = whiteTexture;
#endif
            }
            useFbo = rcUseFbo;
            if (currentColor != rc.color || condTexture) {
                currentColor = rc.color;
                currentEffect =
                    changeShaderProgram(currentEffect, firstCall, (boundTexture != InternalTexture::Invalid), currentColor, camViewPerspMatrix, currentFlags & EnableColorWriteBit);
            }
        }

#if SAC_DEBUG
        batchContent.resize(batchSizes.size() + 1);
        batchContent[batchSizes.size() - 1].push_back(rc);
#endif

        // lookup shape
        const Polygon& polygon = theTransformationSystem.shapes[rc.shapeType];

        if (((batchVertexCount + polygon.vertices.size()) >= MAX_VERTEX_COUNT) | ((indiceCount + polygon.indices.size()) >= MAX_INDICE_COUNT)) {
            #if SAC_DEBUG
            batchSizes.push_back(std::make_pair(BatchFlushReason::Full, batchTriangleCount));
            #endif
            indiceCount = batchTriangleCount = batchVertexCount = drawBatchES2(TEX, vertices, indices, batchVertexCount, batchTriangleCount, indiceCount);
        }

        // ADD TO BATCH
        addRenderCommandToBatch(rc,
            polygon,
            vertices + batchVertexCount,
            indices + indiceCount,
            &batchVertexCount, &batchTriangleCount, &indiceCount);
    }
    #if SAC_DEBUG
    batchSizes.push_back(std::make_pair(BatchFlushReason::End, batchTriangleCount));
    #endif
    drawBatchES2(TEX, vertices, indices, batchVertexCount, batchTriangleCount, indiceCount);

    #undef TEX

    #if SAC_DEBUG
    static unsigned ______debug = 0;
    if ((++______debug % 3000) == 0) {
        ______debug = 0;
        LOGI("Render command size: " << count << ". Drawn using: " << batchSizes.size() << " batches");
        for (unsigned i=0; i<batchSizes.size(); i++) {
            const auto& p = batchSizes[i];
            LOGI("   # batch " << i << ", size: "<< p.second << ", reason: " << p.first);

            const auto& cnt = batchContent[i];
            for (unsigned j=0; j<cnt.size(); j++) {
                const auto& rc = cnt[j];
                if (!rc.e)
                    continue;

                auto tex = RENDERING(rc.e)->texture;
                LOGI("      > rc " << j << "[" << std::hex << rc.key << "]: '" << theEntityManager.entityName(rc.e)
                    << "', z:" << rc.z << ", flags:" << std::hex << rc.flags << std::dec
                    << ", texture: '" << (tex == InvalidTextureRef ? "None" : theRenderingSystem.textureLibrary.ref2Name(tex))
                    << "', color:" << rc.color);
            }
        }
    }
    batchSizes.clear();
    #endif

    LOGT_EVERY_N(600, "Discard Depth attachment");
    /*
        const GLenum discards[]  = {GL_DEPTH_ATTACHMENT};
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glDiscardFramebufferEXT(GL_FRAMEBUFFER,1,discards);
    */
    glState.flags.current = currentFlags;
}

void RenderingSystem::waitDrawingComplete() {
#if ! SAC_EMSCRIPTEN || BENCHMARK_MODE
    PROFILE("Renderer", "wait-drawing-donE", BeginEvent);
    int readQueue = (currentWriteQueue + 1) % 2;
    std::unique_lock<std::mutex> lock(mutexes[L_RENDER]);
    while (renderQueue[readQueue].count > 0 && frameQueueWritable)
        cond[C_RENDER_DONE].wait(lock);
    lock.unlock();
    PROFILE("Renderer", "wait-drawing-donE", EndEvent);
#endif
}

void RenderingSystem::render() {
#if ! SAC_EMSCRIPTEN
    if (!initDone)
        return;
    PROFILE("Renderer", "wait-frame", BeginEvent);

    std::unique_lock<std::mutex> lock(mutexes[L_QUEUE]);
    // processDelayedTextureJobs();
    while (!newFrameReady && frameQueueWritable) {
        cond[C_FRAME_READY].wait(lock);
    }
#endif
    int readQueue = (currentWriteQueue + 1) % 2;
    newFrameReady = false;
    if (!frameQueueWritable) {
        LOGV(1, "Rendering disabled");
        renderQueue[readQueue].count = 0;
#if ! SAC_EMSCRIPTEN
        lock.unlock();
#endif
        return;
    }
#if ! SAC_EMSCRIPTEN
    lock.unlock();
    PROFILE("Renderer", "wait-frame", EndEvent);
#endif
    PROFILE("Renderer", "load-textures", BeginEvent);
    processDelayedTextureJobs();
#if SAC_ENABLE_LOG && ! SAC_EMSCRIPTEN
    //float aftertexture= TimeUtil::GetTime();
#endif
    PROFILE("Renderer", "load-textures", EndEvent);
#if ! SAC_EMSCRIPTEN
    if (!mutexes[L_RENDER].try_lock()) {
        LOGV(1, "HMM Busy render lock");
        mutexes[L_RENDER].lock();
    }
#endif

    PROFILE("Renderer", "render", BeginEvent);
    if (renderQueue[readQueue].count == 0) {
        LOGW("Arg, nothing to render - probably a bug (queue=" << readQueue << ')');
    } else {
        RenderQueue& inQueue = renderQueue[readQueue];
        drawRenderCommands(inQueue);
        inQueue.count = 0;
    }
#if ! SAC_EMSCRIPTEN
    cond[C_RENDER_DONE].notify_all();
    mutexes[L_RENDER].unlock();
#endif
    PROFILE("Renderer", "render", EndEvent);
#if SAC_INGAME_EDITORS
    LevelEditor::lock();
    TwDraw();
    LevelEditor::unlock();
#endif
}

static void computeVerticesScreenPos(const std::vector<glm::vec2>& points, const glm::vec2& position, const glm::vec2& hSize, float rotation, float z, VertexData* out) {
    for (unsigned i=0; i<points.size(); i++) {
        out[i].position = glm::vec3(position + glm::rotate(points[i] * (2.0f * hSize), rotation), z);
    }
}

const Shader& RenderingSystem::effectRefToShader(EffectRef ref, bool firstCall, bool colorEnabled, bool hasTexture) {
    if (ref == DefaultEffectRef) {
        if (colorEnabled) {
            if (firstCall) {
                ref = defaultShaderNoAlpha;
            } else if (hasTexture) {
                ref = defaultShader;
            } else {
                ref = defaultShaderNoTexture;
            }
        } else {
            ref = defaultShaderEmpty;
        }
    }
    return *effectLibrary.get(ref, false);
}
