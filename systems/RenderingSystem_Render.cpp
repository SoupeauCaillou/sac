#include "RenderingSystem.h"
#include "RenderingSystem_Private.h"
#include "base/EntityManager.h"
#include <cmath>
#include <cassert>
#include <sstream>
#include <pthread.h>

void RenderingSystem::check_GL_errors(const char* context) {
	 int maxIterations=10;
    GLenum error;
    while (((error = glGetError()) != GL_NO_ERROR) && maxIterations > 0)
    {
        switch(error)
        {
            case GL_INVALID_ENUM:
            	LOGE("[%2d]GL error: '%s' -> GL_INVALID_ENUM\n", maxIterations, context); break;
            case GL_INVALID_VALUE:
            	LOGE("[%2d]GL error: '%s' -> GL_INVALID_VALUE\n", maxIterations, context); break;
            case GL_INVALID_OPERATION:
            	LOGE("[%2d]GL error: '%s' -> GL_INVALID_OPERATION\n", maxIterations, context); break;
            case GL_OUT_OF_MEMORY:
            	LOGE("[%2d]GL error: '%s' -> GL_OUT_OF_MEMORY\n", maxIterations, context); break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
            	LOGE("[%2d]GL error: '%s' -> GL_INVALID_FRAMEBUFFER_OPERATION\n", maxIterations, context); break;
            default:
            	LOGE("[%2d]GL error: '%s' -> %x\n", maxIterations, context, error);
        }
		  maxIterations--;
    }
}

GLuint RenderingSystem::compileShader(const std::string& assetName, GLuint type) {
    LOGI("Compiling '%s' shader...", assetName.c_str());
	FileBuffer fb = assetAPI->loadAsset(assetName);
	GLuint shader = glCreateShader(type);
	GL_OPERATION(glShaderSource(shader, 1, (const char**)&fb.data, &fb.size))
	GL_OPERATION(glCompileShader(shader))

	delete[] fb.data;
  	GLint logLength;
   GL_OPERATION(glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength))
    if (logLength > 1)
    {
        char *log = new char[logLength];
        GL_OPERATION(glGetShaderInfoLog(shader, logLength, &logLength, log))
        LOGE("GL shader error: %s\n", log);
 		delete[] log;
    }

   if (!glIsShader(shader)) {
   	LOGW("Weird; %d is not a shader", shader);
   }
	return shader;
}

static void computeVerticesScreenPos(const Vector2& position, const Vector2& hSize, float rotation, int rotateUV, Vector2* out);

bool firstCall;
#ifdef USE_VBO
#define DRAW(texture, vert, uv, indices, batchSize, rotateUV) drawBatchES2(texture, rotateUV, batchSize)
static int drawBatchES2(const RenderingSystem::InternalTexture& glref, bool rotateUV, int batchSize) {
#else
#define DRAW(texture, vert, uv, indices, batchSize, reverseUV) drawBatchES2(texture, vert, uv, indices, batchSize)
static int drawBatchES2(const RenderingSystem::InternalTexture& glref, const GLfloat* vertices, const GLfloat* uvs, const unsigned short* indices, int batchSize) {
#endif
    if (batchSize > 0)
    {
    	GL_OPERATION(glActiveTexture(GL_TEXTURE0))
    	// GL_OPERATION(glEnable(GL_TEXTURE_2D)
    	GL_OPERATION(glBindTexture(GL_TEXTURE_2D, glref.color))

    	// GL_OPERATION(glEnable(GL_TEXTURE_2D))
    	if (firstCall) {
    		// GL_OPERATION(glBindTexture(GL_TEXTURE_2D, 0))
    	} else {
            GL_OPERATION(glActiveTexture(GL_TEXTURE1))
    		GL_OPERATION(glBindTexture(GL_TEXTURE_2D, glref.alpha))
    	}

    #ifdef USE_VBO
    	GL_OPERATION(glBindBuffer(GL_ARRAY_BUFFER, theRenderingSystem.squareBuffers[rotateUV ? 1 : 0]))

    	GL_OPERATION(glEnableVertexAttribArray(ATTRIB_VERTEX))
    	GL_OPERATION(glEnableVertexAttribArray(ATTRIB_UV))
    	GL_OPERATION(glVertexAttribPointer(ATTRIB_VERTEX, 3, GL_FLOAT, 0, 5 * sizeof(float), 0))
    	GL_OPERATION(glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, 0, 5 * sizeof(float), (float*) 0 + 3))

    	GL_OPERATION(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theRenderingSystem.squareBuffers[2]))
    	GL_OPERATION(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0))
    #else
    	GL_OPERATION(glVertexAttribPointer(ATTRIB_VERTEX, 3, GL_FLOAT, 0, 0, vertices))
    	GL_OPERATION(glEnableVertexAttribArray(ATTRIB_VERTEX))
    	GL_OPERATION(glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, 1, 0, uvs))
    	GL_OPERATION(glEnableVertexAttribArray(ATTRIB_UV))

    	GL_OPERATION(glDrawElements(GL_TRIANGLES, batchSize * 6, GL_UNSIGNED_SHORT, indices))
#endif
    }
    return 0;
}

#ifdef USE_VBO
static inline void computeUV(RenderingSystem::RenderCommand& rc, const RenderingSystem::TextureInfo& info, GLint unif) {
#else
static inline void computeUV(RenderingSystem::RenderCommand& rc, const RenderingSystem::TextureInfo& info) {
#endif
     rc.glref = info.glref;
     Vector2 offset(rc.uv[0]);
     Vector2 scale(rc.uv[1]);
     const Vector2 uvS (info.uv[1] - info.uv[0]);

     if (info.rotateUV) {
         std::swap(offset.X, offset.Y);
         std::swap(scale.X, scale.Y);
         offset.Y = 1 - (scale.Y + offset.Y);
     }
     {
         rc.uv[0] = info.uv[0] + Vector2(offset.X * uvS.X, offset.Y * uvS.Y);
         rc.uv[1] = rc.uv[0] + Vector2(scale.X * uvS.X, scale.Y * uvS.Y);
     }
    if (rc.mirrorH) {
        if (info.rotateUV)
            std::swap(rc.uv[0].Y, rc.uv[1].Y);
        else
            std::swap(rc.uv[0].X, rc.uv[1].X);
    }
    rc.rotateUV = info.rotateUV;
     #ifdef USE_VBO
     float uvso[4];
     uvso[0] = rc.uv[1].X - rc.uv[0].X;
     uvso[1] = rc.uv[1].Y - rc.uv[0].Y;
     uvso[2] = rc.uv[0].X;
     uvso[3] = rc.uv[0].Y;
     GL_OPERATION(glUniform4fv(unif, 1, uvso))
     #endif
}

#ifdef USE_VBO
static inline void addRenderCommandToBatch(float screenW, float screenH, const RenderingSystem::RenderCommand& rc, const RenderingSystem::Camera& camera, const RenderingSystem::Shader& shader) {
#else
static inline void addRenderCommandToBatch(const RenderingSystem::RenderCommand& rc, int batchSize, GLfloat* vertices, GLfloat* uvs, unsigned short* indices) {
#endif
    #ifdef USE_VBO
    float hW = 0.5 * screenW;
    float hH = 0.5 * screenH;
    GLfloat mat[16];
    RenderingSystem::loadOrthographicMatrix(
        -hW - rc.position.X + camera.worldPosition.X,
        hW - rc.position.X + camera.worldPosition.X,
        -hH - rc.position.Y + camera.worldPosition.Y,
        hH - rc.position.Y + camera.worldPosition.Y,
        0, 1, mat);
    GL_OPERATION(glUniform1f(shader.uniformRotation, -rc.rotation))
    float scaleZ[] = { 2 * rc.halfSize.X, 2 * rc.halfSize.Y, rc.z };
    GL_OPERATION(glUniform3fv(shader.uniformScaleZ, 1, scaleZ))
    GL_OPERATION(glUniformMatrix4fv(shader.uniformMatrix, 1, GL_FALSE, mat))
    #else
    // fill batch
    Vector2 onScreenVertices[4];
    computeVerticesScreenPos(rc.position, rc.halfSize, rc.rotation, rc.rotateUV, onScreenVertices);

    const int baseIdx = 4 * batchSize;
    for (int i=0; i<4; i++) {
        vertices[(baseIdx + i) * 3 + 0] = onScreenVertices[i].X;
        vertices[(baseIdx + i) * 3 + 1] = onScreenVertices[i].Y;
        vertices[(baseIdx + i) * 3 + 2] = -rc.z;
    }

    uvs[baseIdx * 2 + 0] = rc.uv[0].X;
    uvs[baseIdx * 2 + 1] = 1-rc.uv[0].Y;
    uvs[baseIdx * 2 + 2] = rc.uv[1].X;
    uvs[baseIdx * 2 + 3] = 1-rc.uv[0].Y;
    uvs[baseIdx * 2 + 4] = rc.uv[0].X;
    uvs[baseIdx * 2 + 5] = 1-rc.uv[1].Y;
    uvs[baseIdx * 2 + 6] = rc.uv[1].X;
    uvs[baseIdx * 2 + 7] = 1-rc.uv[1].Y;

    indices[batchSize * 6 + 0] = baseIdx + 0;
    indices[batchSize * 6 + 1] = baseIdx + 1;
    indices[batchSize * 6 + 2] = baseIdx + 2;
    indices[batchSize * 6 + 3] = baseIdx + 1;
    indices[batchSize * 6 + 4] = baseIdx + 3;
    indices[batchSize * 6 + 5] = baseIdx + 2;
    #endif
}

EffectRef RenderingSystem::changeShaderProgram(EffectRef ref, bool _firstCall, const Color& color, const Camera& camera, bool colorEnabled) {
	const Shader& shader = effectRefToShader(ref, _firstCall, colorEnabled);
	GL_OPERATION(glUseProgram(shader.program))
	GLfloat mat[16];

#ifndef USE_VBO
    const float left = (-camera.worldSize.X * 0.5 + camera.worldPosition.X);
    const float right = (camera.worldSize.X * 0.5 + camera.worldPosition.X);
    const float bottom = (-camera.worldSize.Y * 0.5 + camera.worldPosition.Y);
    const float top = (camera.worldSize.Y * 0.5 + camera.worldPosition.Y);
	loadOrthographicMatrix(left, right, camera.mirrorY ? top : bottom, camera.mirrorY ? bottom : top, 0, 1, mat);
	GL_OPERATION(glUniform1fv(shader.uniformMatrix, 6, mat))
#endif

	GL_OPERATION(glUniform1i(shader.uniformColorSampler, 0))
	GL_OPERATION(glUniform1i(shader.uniformAlphaSampler, 1))
	GL_OPERATION(glUniform4fv(shader.uniformColor, 1, color.rgba))
	return ref;
}

void RenderingSystem::drawRenderCommands(RenderQueue& commands) {
#ifdef USE_VBO
#define MAX_BATCH_SIZE 1
#else
#define MAX_BATCH_SIZE 128
#endif
	static GLfloat vertices[MAX_BATCH_SIZE * 4 * 3];
	static GLfloat uvs[MAX_BATCH_SIZE * 4 * 2];
	static unsigned short indices[MAX_BATCH_SIZE * 6];
    Camera camera;
	int batchSize = 0;
    GL_OPERATION(glDepthMask(true))
    GL_OPERATION(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
    GL_OPERATION(glEnable(GL_DEPTH_TEST))
	InternalTexture boundTexture = InternalTexture::Invalid;
	Color currentColor(1,1,1,1);
    GL_OPERATION(glActiveTexture(GL_TEXTURE1))
    GL_OPERATION(glBindTexture(GL_TEXTURE_2D, 0))
    int currentFlags = 0;

	EffectRef currentEffect = InvalidTextureRef;
    const unsigned count = commands.count;
    for (unsigned i=0; i<count; i++) {
		RenderCommand& rc = commands.commands[i];

        // HANDLE BEGIN/END FRAME MARKERS
        if (rc.texture == BeginFrameMarker) {
            batchSize = DRAW(boundTexture, vertices, uvs, indices, batchSize, false);

            #ifdef ENABLE_PROFILING
            std::stringstream framename;
            framename << "render-frame-" << (unsigned int)rc.rotateUV;
            PROFILE("Render", framename.str(), InstantEvent);
            #endif

            firstCall = true;
            camera.worldPosition = rc.halfSize;
            camera.worldSize = rc.uv[0];
            camera.screenPosition = rc.uv[1];
            camera.screenSize = rc.position;
            camera.mirrorY = rc.effectRef;

			GL_OPERATION(glViewport(
		        (camera.screenPosition.X - camera.screenSize.X * 0.5 + 0.5) * windowW,
		        (camera.screenPosition.Y - camera.screenSize.Y * 0.5 + 0.5) * windowH,
		        windowW * camera.screenSize.X, windowH * camera.screenSize.Y))

            // setup initial GL state
            currentEffect = changeShaderProgram(DefaultEffectRef, firstCall, currentColor, camera);
            GL_OPERATION(glDepthMask(true))
            GL_OPERATION(glDisable(GL_BLEND))
            GL_OPERATION(glColorMask(true, true, true, true))
            currentFlags = (EnableZWriteBit | DisableBlendingBit | EnableColorWriteBit);
            continue;
        } else if (rc.texture == EndFrameMarker) {
			break;
		}

        // HANDLE RENDERING FLAGS (GL state switch)
        if (rc.flags != currentFlags) {
            // flush batch before changing state
            batchSize = DRAW(boundTexture, vertices, uvs, indices, batchSize, false);
            if (rc.flags & EnableZWriteBit) {
                GL_OPERATION(glDepthMask(true))
            } else if (rc.flags & DisableZWriteBit) {
                GL_OPERATION(glDepthMask(false))
            } if (rc.flags & EnableBlendingBit) {
                firstCall = false;
                GL_OPERATION(glEnable(GL_BLEND))
                if (currentEffect == DefaultEffectRef) {
                    currentEffect = changeShaderProgram(DefaultEffectRef, firstCall, currentColor, camera);
                }
            } else if (rc.flags & DisableBlendingBit) {
                 GL_OPERATION(glDisable(GL_BLEND))
            } if (rc.flags & EnableColorWriteBit) {
                GL_OPERATION(glColorMask(true, true, true, true))
                if (!(currentFlags & EnableColorWriteBit)) {
                    if (currentEffect == DefaultEffectRef) {
                        currentEffect = changeShaderProgram(DefaultEffectRef, firstCall, currentColor, camera);
                    }
                }
            } else if (rc.flags & DisableColorWriteBit) {
                GL_OPERATION(glColorMask(false, false, false, false))
                if (!(currentFlags & DisableColorWriteBit)) {
                    if (currentEffect == DefaultEffectRef) {
                        currentEffect = changeShaderProgram(DefaultEffectRef, firstCall, currentColor, camera, false);
                    }
                }
            }
            currentFlags = rc.flags;
        }
        // EFFECT HAS CHANGED ?
		if (rc.effectRef != currentEffect) {
            // flush before changing effect
			batchSize = DRAW(boundTexture, vertices, uvs, indices, batchSize, false);
			currentEffect = changeShaderProgram(rc.effectRef, firstCall, currentColor, camera, currentFlags & EnableColorWriteBit);
		}

        // SETUP TEXTURING
		if (rc.texture != InvalidTextureRef) {
            const TextureInfo& info = textures[rc.texture];
            #ifdef USE_VBO
            computeUV(rc, info, effectRefToShader(currentEffect, firstCall, currentFlags & EnableColorWriteBit).uniformUVScaleOffset);
            #else
            computeUV(rc, info);
            #endif
		} else {
			rc.glref = InternalTexture::Invalid;
			rc.glref.color = whiteTexture;
			rc.glref.alpha = whiteTexture;
			rc.uv[0] = Vector2::Zero;
			rc.uv[1] = Vector2(1,1);
			rc.rotateUV = 0;
			#ifdef USE_VBO
			float uvso[4];
			uvso[0] = uvso[1] = 1;
			uvso[2] = uvso[3] = 0;
			GL_OPERATION(glUniform4fv(effectRefToShader(currentEffect, firstCall, currentFlags & EnableColorWriteBit).uniformUVScaleOffset, 1, uvso))
			#endif
		}

        // TEXTURE OR COLOR HAS CHANGED ?
		if (boundTexture != rc.glref || currentColor != rc.color) {
            // flush before changing texture/color
			batchSize = DRAW(boundTexture, vertices, uvs, indices, batchSize, false);
			boundTexture = rc.glref;
			if (currentColor != rc.color) {
	            currentColor = rc.color;
	            GL_OPERATION(glUniform4fv(effectRefToShader(currentEffect, firstCall, currentFlags & EnableColorWriteBit).uniformColor, 1, currentColor.rgba))
			}
		}

        // ADD TO BATCH
        #ifdef USE_VBO
		addRenderCommandToBatch(screenW, screenH, rc, camera, effectRefToShader(currentEffect, firstCall, currentFlags & EnableColorWriteBit));
        #else
        addRenderCommandToBatch(rc, batchSize, vertices, uvs, indices);
        #endif
		batchSize++;

		if (batchSize == MAX_BATCH_SIZE) {
            batchSize = DRAW(boundTexture, vertices, uvs, indices, batchSize, rc.rotateUV);
		}
	}
    batchSize = DRAW(boundTexture, vertices, uvs, indices, batchSize, false);
}
#include <errno.h>

void RenderingSystem::waitDrawingComplete() {
#ifndef EMSCRIPTEN
    PROFILE("Renderer", "wait-drawing-donE", BeginEvent);
    int readQueue = (currentWriteQueue + 1) % 2;
    pthread_mutex_lock(&mutexes[L_RENDER]);
    while (renderQueue[readQueue].count > 0 && frameQueueWritable)
        pthread_cond_wait(&cond[C_RENDER_DONE], &mutexes[L_RENDER]);
    pthread_mutex_unlock(&mutexes[L_RENDER]);
    PROFILE("Renderer", "wait-drawing-donE", EndEvent);
#endif
}

void RenderingSystem::render() {
#ifdef ENABLE_LOG
    float enter = TimeUtil::getTime();
#endif
#ifndef EMSCRIPTEN
    PROFILE("Renderer", "wait-frame", BeginEvent);
    pthread_mutex_lock(&mutexes[L_QUEUE]);
    while (!newFrameReady && frameQueueWritable) {
        pthread_cond_wait(&cond[C_FRAME_READY], &mutexes[L_QUEUE]);
    }
#endif
#if defined(ENABLE_LOG) && !defined(EMSCRIPTEN)
    //float frameready = TimeUtil::getTime();
#endif
    if (!frameQueueWritable) {
        LOGI("Rendering disabled");
        #ifndef EMSCRIPTEN
        pthread_mutex_unlock(&mutexes[L_QUEUE]);
        #endif
        return;
    }
    newFrameReady = false;
    int readQueue = (currentWriteQueue + 1) % 2;
#ifndef EMSCRIPTEN
    pthread_mutex_unlock(&mutexes[L_QUEUE]);
    PROFILE("Renderer", "wait-frame", EndEvent);
#endif
    PROFILE("Renderer", "load-textures", BeginEvent);
    processDelayedTextureJobs();
#if defined(ENABLE_LOG) && !defined(EMSCRIPTEN)
    //float aftertexture= TimeUtil::getTime();
#endif
    PROFILE("Renderer", "load-textures", EndEvent);
#ifndef EMSCRIPTEN
	if (pthread_mutex_trylock(&mutexes[L_RENDER]) != 0) {
		LOGW("HMM Busy render lock");
		pthread_mutex_lock(&mutexes[L_RENDER]);
	}
#endif
#if defined(ENABLE_LOG) && !defined(EMSCRIPTEN)
    float ppp = TimeUtil::getTime();
    float diff =  ppp - enter;
    if (diff > 0.001) {
        //- LOGI("Diff : %.1f ms / %.1f ms -> %.1f ms -> %.1f ms",
            //- diff * 1000,
            //- 1000 * (frameready - enter),
            //- 1000 * (aftertexture - frameready),
            //- 1000 * (ppp - aftertexture));
    }
#endif
    PROFILE("Renderer", "render", BeginEvent);
    if (renderQueue[readQueue].count == 0) {
        LOGW("Arg, nothing to render - probably a bug (queue=%d)", readQueue);
    } else {
        RenderQueue& inQueue = renderQueue[readQueue];
        drawRenderCommands(inQueue);
        inQueue.count = 0;
    }
#ifndef EMSCRIPTEN
    pthread_cond_signal(&cond[C_RENDER_DONE]);
    pthread_mutex_unlock(&mutexes[L_RENDER]);
#endif
	PROFILE("Renderer", "render", EndEvent);
}

static void computeVerticesScreenPos(const Vector2& position, const Vector2& hSize, float rotation, int rotateUV, Vector2* out) {
	const float cr = cos(rotation);
	const float sr = -sin(rotation);

	const float crX = cr * hSize.X;
	const float crY = cr * hSize.Y;
	const float srX = sr * hSize.X;
	const float srY = sr * hSize.Y;

	// -x -y
	out[rotateUV ? 2 : 0] = Vector2(-crX - srY + position.X,  -(-srX) - crY + position.Y);
	// +x -y
	out[rotateUV ? 0 : 1] = Vector2(crX - srY + position.X, -srX - crY + position.Y);
	// -x +y
	out[rotateUV ? 3 : 2] = Vector2(-crX + srY + position.X, -(-srX) + crY + position.Y);
	// +x +y
	out[rotateUV ? 1 : 3] = Vector2(crX + srY + position.X, -srX + crY + position.Y);
}

void RenderingSystem::loadOrthographicMatrix(float left, float right, float bottom, float top, float near, float far, float* mat)
{
    float r_l = right - left;
    float t_b = top - bottom;
    float f_n = far - near;
    float tx = - (right + left) / r_l;
    float ty = - (top + bottom) / t_b;
    float tz = - (far + near) / f_n;

#ifndef USE_VBO
    mat[0] = 2.0f / r_l;
    mat[1] = 2.0f / t_b;
    mat[2] = -2.0f / f_n;
    mat[3] = tx;
    mat[4] = ty;
    mat[5] = tz;
#else
    mat[0] = 2.0f / r_l;
    mat[1] = mat[2] = mat[3] = 0.0f;

    mat[4] = 0.0f;
    mat[5] = 2.0f / t_b;
    mat[6] = mat[7] = 0.0f;

    mat[8] = mat[9] = 0.0f;
    mat[10] = -2.0f / f_n;
    mat[11] = 0.0f;

    mat[12] = tx;
    mat[13] = ty;
    mat[14] = tz;
    mat[15] = 1.0f;
#endif
}

const RenderingSystem::Shader& RenderingSystem::effectRefToShader(EffectRef ref, bool firstCall, bool colorEnabled) {
	if (ref == DefaultEffectRef) {
		return colorEnabled ? (firstCall ? defaultShaderNoAlpha : defaultShader) : defaultShaderEmpty;
	} else {
		return ref2Effects[ref];
	}
}
