#include "RenderingSystem.h"
#include "RenderingSystem_Private.h"
#include "CameraSystem.h"
#include "TransformationSystem.h"
#include "base/EntityManager.h"
#include <cmath>
#include <sstream>
#if SAC_INGAME_EDITORS
#include <AntTweakBar.h>
#include "util/LevelEditor.h"
#endif
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

static void computeVerticesScreenPos(const glm::vec2& position, const glm::vec2& hSize, float rotation, int rotateUV, glm::vec2* out);

bool firstCall;

RenderingSystem::ColorAlphaTextures RenderingSystem::chooseTextures(const InternalTexture& tex, const FramebufferRef& fbo, bool useFbo) {
    if (useFbo) {
        RenderingSystem::Framebuffer b = ref2Framebuffers[fbo];
        return std::make_pair(b.texture, b.texture);
    } else {
        return std::make_pair(tex.color, tex.alpha);
    }
}

#define DRAW(texture, vert, uv, batchSize, reverseUV) drawBatchES2(texture, vert, uv, batchSize)
static int drawBatchES2(const RenderingSystem::ColorAlphaTextures glref, const GLfloat* vertices, const GLfloat* uvs, int batchSize) {
    if (batchSize > 0)
    {
    	GL_OPERATION(glActiveTexture(GL_TEXTURE0))
    	// GL_OPERATION(glEnable(GL_TEXTURE_2D)
    	GL_OPERATION(glBindTexture(GL_TEXTURE_2D, glref.first))

    	// GL_OPERATION(glEnable(GL_TEXTURE_2D))
    	if (firstCall) {
    		// GL_OPERATION(glBindTexture(GL_TEXTURE_2D, 0))
    	} else {
            GL_OPERATION(glActiveTexture(GL_TEXTURE1))
    		GL_OPERATION(glBindTexture(GL_TEXTURE_2D, glref.second))
    	}

#if SAC_USE_VBO
        // update vertex buffer
    	GL_OPERATION(glBindBuffer(GL_ARRAY_BUFFER, theRenderingSystem.squareBuffers[1]))
        GL_OPERATION(glBufferSubData(GL_ARRAY_BUFFER, 0,
            batchSize * 4 * 3 * sizeof(float), vertices))
        GL_OPERATION(glEnableVertexAttribArray(EffectLibrary::ATTRIB_VERTEX))
        GL_OPERATION(glVertexAttribPointer(EffectLibrary::ATTRIB_VERTEX, 3, GL_FLOAT, 0, 3 * sizeof(float), 0))

        // update uv buffer
        GL_OPERATION(glBindBuffer(GL_ARRAY_BUFFER, theRenderingSystem.squareBuffers[2]))
        GL_OPERATION(glBufferSubData(GL_ARRAY_BUFFER, 0,
            batchSize * 4 * 2 * sizeof(float), uvs))
        GL_OPERATION(glEnableVertexAttribArray(EffectLibrary::ATTRIB_UV))
        GL_OPERATION(glVertexAttribPointer(EffectLibrary::ATTRIB_UV, 2, GL_FLOAT, 0, 2 * sizeof(float), 0))

        // indices buffer
        GL_OPERATION(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theRenderingSystem.squareBuffers[0]))

    	GL_OPERATION(glDrawElements(GL_TRIANGLES, batchSize * 6, GL_UNSIGNED_SHORT, 0))
#else
    	GL_OPERATION(glVertexAttribPointer(EffectLibrary::ATTRIB_VERTEX, 3, GL_FLOAT, 0, 0, vertices))
    	GL_OPERATION(glEnableVertexAttribArray(EffectLibrary::ATTRIB_VERTEX))
    	GL_OPERATION(glVertexAttribPointer(EffectLibrary::ATTRIB_UV, 2, GL_FLOAT, 1, 0, uvs))
    	GL_OPERATION(glEnableVertexAttribArray(EffectLibrary::ATTRIB_UV))

    	GL_OPERATION(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theRenderingSystem.squareBuffers[0]))

        GL_OPERATION(glDrawElements(GL_TRIANGLES, batchSize * 6, GL_UNSIGNED_SHORT, 0))
#endif
    }
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
    if (rc.mirrorH) {
        if (info.rotateUV)
            std::swap(rc.uv[0].y, rc.uv[1].y);
        else
            std::swap(rc.uv[0].x, rc.uv[1].x);
    }
    rc.rotateUV = info.rotateUV;
}

static inline void addRenderCommandToBatch(const RenderingSystem::RenderCommand& rc, int batchSize, GLfloat* vertices, GLfloat* uvs) {
    // fill batch
    glm::vec2 onScreenVertices[4];
    computeVerticesScreenPos(rc.position, rc.halfSize, rc.rotation, rc.rotateUV, onScreenVertices);

    const int baseIdx = 4 * batchSize;
    for (int i=0; i<4; i++) {
        vertices[(baseIdx + i) * 3 + 0] = onScreenVertices[i].x;
        vertices[(baseIdx + i) * 3 + 1] = onScreenVertices[i].y;
        vertices[(baseIdx + i) * 3 + 2] = -rc.z;
    }

    uvs[baseIdx * 2 + 0] = rc.uv[0].x;
    uvs[baseIdx * 2 + 1] = 1-rc.uv[0].y;
    uvs[baseIdx * 2 + 2] = rc.uv[1].x;
    uvs[baseIdx * 2 + 3] = 1-rc.uv[0].y;
    uvs[baseIdx * 2 + 4] = rc.uv[0].x;
    uvs[baseIdx * 2 + 5] = 1-rc.uv[1].y;
    uvs[baseIdx * 2 + 6] = rc.uv[1].x;
    uvs[baseIdx * 2 + 7] = 1-rc.uv[1].y;
}

EffectRef RenderingSystem::changeShaderProgram(EffectRef ref, bool _firstCall, const Color& color, const glm::mat4& mvp, bool colorEnabled) {
	const Shader& shader = effectRefToShader(ref, _firstCall, colorEnabled);
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
	static GLfloat vertices[MAX_BATCH_SIZE * 4 * 3];
	static GLfloat uvs[MAX_BATCH_SIZE * 4 * 2];
    struct {
        TransformationComponent worldPos;
        CameraComponent cameraAttr;
    } camera;
	int batchSize = 0;
    GL_OPERATION(glDepthMask(true))

    GL_OPERATION(glEnable(GL_DEPTH_TEST))
	InternalTexture boundTexture = InternalTexture::Invalid;
    FramebufferRef fboRef = DefaultFrameBufferRef;
	Color currentColor(1,1,1,1);
    GL_OPERATION(glActiveTexture(GL_TEXTURE1))
    GL_OPERATION(glBindTexture(GL_TEXTURE_2D, 0))
    int currentFlags = 0;
    bool useFbo = false;

    // matrices
    glm::mat4 camViewPerspMatrix;

    LOGV(2, "Begin frame rendering: " << commands.count)

	EffectRef currentEffect = InvalidTextureRef;
    const unsigned count = commands.count;
    for (unsigned i=0; i<count; i++) {
		RenderCommand& rc = commands.commands[i];

        // HANDLE BEGIN/END FRAME MARKERS
        if (rc.texture == BeginFrameMarker) {
            batchSize = DRAW(chooseTextures(boundTexture, fboRef, useFbo), vertices, uvs, batchSize, false);

#if SAC_ENABLE_PROFILING
            std::stringstream framename;
            framename << "render-frame-" << (unsigned int)rc.rotateUV;
            PROFILE("Render", framename.str(), InstantEvent);
#endif

            firstCall = true;
            unpackCameraAttributes(rc, &camera.worldPos, &camera.cameraAttr);
            LOGV(2, "   camera: pos=" << camera.worldPos.position.x << ',' << camera.worldPos.position.y
                << "size=" << camera.worldPos.size.x << ',' << camera.worldPos.size.y
                << " fb=" << camera.cameraAttr.fb)

            FramebufferRef fboRef = camera.cameraAttr.fb;
            if (fboRef == DefaultFrameBufferRef) {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                GL_OPERATION(glViewport(0, 0, windowW, windowH))
            } else {
                const Framebuffer& fb = ref2Framebuffers[fboRef];
                glBindFramebuffer(GL_FRAMEBUFFER, fb.fbo);
                GL_OPERATION(glViewport(0, 0, fb.width, fb.height))
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
            currentEffect = changeShaderProgram(DefaultEffectRef, firstCall, currentColor, camViewPerspMatrix);
            GL_OPERATION(glDepthMask(true))
            GL_OPERATION(glDisable(GL_BLEND))
            GL_OPERATION(glColorMask(true, true, true, true))
            GL_OPERATION(glClearColor(camera.cameraAttr.clearColor.r, camera.cameraAttr.clearColor.g, camera.cameraAttr.clearColor.b, camera.cameraAttr.clearColor.a))
            GL_OPERATION(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
            currentFlags = (EnableZWriteBit | DisableBlendingBit | EnableColorWriteBit);
            continue;
        } else if (rc.texture == EndFrameMarker) {
			break;
		}

        // HANDLE RENDERING FLAGS (GL state switch)
        if (rc.flags != currentFlags) {
            // flush batch before changing state
            batchSize = DRAW(chooseTextures(boundTexture, fboRef, useFbo), vertices, uvs, batchSize, false);
            if (rc.flags & EnableZWriteBit) {
                GL_OPERATION(glDepthMask(true))
            } else if (rc.flags & DisableZWriteBit) {
                GL_OPERATION(glDepthMask(false))
            } if (rc.flags & EnableBlendingBit) {
                firstCall = false;
                GL_OPERATION(glEnable(GL_BLEND))
                if (currentEffect == DefaultEffectRef) {
                    currentEffect = changeShaderProgram(DefaultEffectRef, firstCall, currentColor, camViewPerspMatrix);
                }
            } else if (rc.flags & DisableBlendingBit) {
                 GL_OPERATION(glDisable(GL_BLEND))
            } if (rc.flags & EnableColorWriteBit) {
                GL_OPERATION(glColorMask(true, true, true, true))
                if (!(currentFlags & EnableColorWriteBit)) {
                    if (currentEffect == DefaultEffectRef) {
                        currentEffect = changeShaderProgram(DefaultEffectRef, firstCall, currentColor, camViewPerspMatrix);
                    }
                }
            } else if (rc.flags & DisableColorWriteBit) {
                GL_OPERATION(glColorMask(false, false, false, false))
                if (!(currentFlags & DisableColorWriteBit)) {
                    if (currentEffect == DefaultEffectRef) {
                        currentEffect = changeShaderProgram(DefaultEffectRef, firstCall, currentColor, camViewPerspMatrix, false);
                    }
                }
            }
            currentFlags = rc.flags;
        }
        // EFFECT HAS CHANGED ?
		if (rc.effectRef != currentEffect) {
            // flush before changing effect
			batchSize = DRAW(chooseTextures(boundTexture, fboRef, useFbo), vertices, uvs, batchSize, false);
			currentEffect = changeShaderProgram(rc.effectRef, firstCall, currentColor, camViewPerspMatrix, currentFlags & EnableColorWriteBit);
		}

        // SETUP TEXTURING
		if (rc.texture != InvalidTextureRef) {
            if (!rc.fbo) {
                const TextureInfo* info = textureLibrary.get(rc.texture, false);
                LOGF_IF(info == 0, "Invalid texture " << rc.texture << " : can not be found")
                if (info->atlasIndex >= 0) {
                    LOGF_IF((unsigned)info->atlasIndex >= atlas.size(), "Invalid atlas index: " << info->atlasIndex << " >= atlas count : " << atlas.size())
                    const TextureInfo* atlasInfo = textureLibrary.get(atlas[info->atlasIndex].ref, false);
                    LOGF_IF(!atlasInfo, "TextureInfo for atlas index: "
                        << info->atlasIndex << " not found (ref=" << atlas[info->atlasIndex].ref << ", name='" << atlas[info->atlasIndex].name << "')")
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
			rc.glref.color = whiteTexture;
			rc.glref.alpha = whiteTexture;
			rc.uv[0] = glm::vec2(0.0f, 0.0f);
			rc.uv[1] = glm::vec2(1.0f, 1.0f);
			rc.rotateUV = 0;
		}

        // TEXTURE OR COLOR HAS CHANGED ?
		if (useFbo != rc.fbo || (!rc.fbo && boundTexture != rc.glref) || (rc.fbo && fboRef != rc.framebuffer) || currentColor != rc.color) {
            // flush before changing texture/color
			batchSize = DRAW(chooseTextures(boundTexture, fboRef, useFbo), vertices, uvs, batchSize, false);
            if (rc.fbo) {
                fboRef = rc.framebuffer;
                boundTexture = InternalTexture::Invalid;
            } else {
                fboRef = DefaultFrameBufferRef;
                boundTexture = rc.glref;
            }
			useFbo = rc.fbo;
			if (currentColor != rc.color) {
	            currentColor = rc.color;
	            GL_OPERATION(glUniform4fv(effectRefToShader(currentEffect, firstCall, currentFlags & EnableColorWriteBit).uniformColor, 1, currentColor.rgba))
			}
		}

        // ADD TO BATCH
        addRenderCommandToBatch(rc, batchSize, vertices, uvs);
		batchSize++;

		if (batchSize == MAX_BATCH_SIZE) {
            batchSize = DRAW(chooseTextures(boundTexture, fboRef, useFbo), vertices, uvs, batchSize, rc.rotateUV);
		}
	}
    batchSize = DRAW(chooseTextures(boundTexture, fboRef, useFbo), vertices, uvs, batchSize, false);
}
#include <errno.h>

void RenderingSystem::waitDrawingComplete() {
#if ! SAC_EMSCRIPTEN
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
        LOGI("Rendering disabled")
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
        LOGV(1, "HMM Busy render lock")
        mutexes[L_RENDER].lock();
    }
#endif

    PROFILE("Renderer", "render", BeginEvent);
    if (renderQueue[readQueue].count == 0) {
        LOGW("Arg, nothing to render - probably a bug (queue=" << readQueue << ')')
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

static void computeVerticesScreenPos(const glm::vec2& position, const glm::vec2& hSize, float rotation, int rotateUV, glm::vec2* out) {
	const float cr = cos(rotation);
	const float sr = -sin(rotation);

	const float crX = cr * hSize.x;
	const float crY = cr * hSize.y;
	const float srX = sr * hSize.x;
	const float srY = sr * hSize.y;

	// -x -y
	out[rotateUV ? 2 : 0] = glm::vec2(-crX - srY + position.x,  -(-srX) - crY + position.y);
	// +x -y
	out[rotateUV ? 0 : 1] = glm::vec2(crX - srY + position.x, -srX - crY + position.y);
	// -x +y
	out[rotateUV ? 3 : 2] = glm::vec2(-crX + srY + position.x, -(-srX) + crY + position.y);
	// +x +y
	out[rotateUV ? 1 : 3] = glm::vec2(crX + srY + position.x, -srX + crY + position.y);
}

const Shader& RenderingSystem::effectRefToShader(EffectRef ref, bool firstCall, bool colorEnabled) {
	if (ref == DefaultEffectRef) {
		ref = (colorEnabled ? (firstCall ? defaultShaderNoAlpha : defaultShader) : defaultShaderEmpty);
	}
	return *effectLibrary.get(ref, false);
}
