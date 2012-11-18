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
static void drawBatchES2(const RenderingSystem::InternalTexture& glref, bool reverseUV) {
#else
static void drawBatchES2(const RenderingSystem::InternalTexture& glref, const GLfloat* vertices, const GLfloat* uvs, const unsigned short* indices, int batchSize) {
#endif
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
	GL_OPERATION(glBindBuffer(GL_ARRAY_BUFFER, theRenderingSystem.squareBuffers[reverseUV ? 1 : 0]))

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

EffectRef RenderingSystem::changeShaderProgram(EffectRef ref, bool _firstCall, const Color& color, const Camera& camera) {
	const Shader& shader = effectRefToShader(ref, _firstCall);
	GL_OPERATION(glUseProgram(shader.program))
	GLfloat mat[16];
 
    const float left = (-camera.worldSize.X * 0.5 + camera.worldPosition.X);
    const float right = (camera.worldSize.X * 0.5 + camera.worldPosition.X);
    const float bottom = (-camera.worldSize.Y * 0.5 + camera.worldPosition.Y);
    const float top = (camera.worldSize.Y * 0.5 + camera.worldPosition.Y);
	loadOrthographicMatrix(left, right, camera.mirrorY ? top : bottom, camera.mirrorY ? bottom : top, 0, 1, mat);
	GL_OPERATION(glUniform1fv(shader.uniformMatrix, 6, mat))

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
	InternalTexture boundTexture = InternalTexture::Invalid, t;
	Color currentColor(1,1,1,1);
    GL_OPERATION(glActiveTexture(GL_TEXTURE1))
    GL_OPERATION(glBindTexture(GL_TEXTURE_2D, 0))

	EffectRef currentEffect = InvalidTextureRef;
    const unsigned count = commands.count;
    for (unsigned i=0; i<count; i++) {
		RenderCommand& rc = commands.commands[i];
        if (rc.texture == BeginFrameMarker) {
            if (batchSize > 0) {
                // execute batch
                #ifdef USE_VBO
                LOGE("Error batching unsupported with VBO");
                #else
                drawBatchES2(boundTexture, vertices, uvs, indices, batchSize);
                #endif
                batchSize = 0;
            }
         
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
        
            currentEffect = changeShaderProgram(DefaultEffectRef, firstCall, currentColor, camera);
            GL_OPERATION(glDepthMask(true))
            GL_OPERATION(glDisable(GL_BLEND))
            
            std::stringstream framename;
            framename << "render-frame-" << (unsigned int)rc.rotateUV;
            PROFILE("Render", framename.str(), InstantEvent);
    
            continue;
        } else if (rc.texture == EndFrameMarker) {
			// LOGW("Frame drawn: %u", rc.rotateUV);
			break;
		}
		else if (rc.texture == DisableZWriteMarker) {
			if (batchSize > 0) {
				// execute batch
				#ifdef USE_VBO
				LOGE("Error batching unsupported with VBO");
				#else
				drawBatchES2(boundTexture, vertices, uvs, indices, batchSize);
				#endif
				batchSize = 0;
			}
            GL_OPERATION(glDepthMask(false))
            continue;
        } else if (rc.texture == EnableBlending) {
            if (batchSize > 0) {
                 // execute batch
                 #ifdef USE_VBO
                 LOGE("Error batching unsupported with VBO");
                 #else
                 drawBatchES2(boundTexture, vertices, uvs, indices, batchSize);
                 #endif
                 batchSize = 0;
            }
			firstCall = false;
			GL_OPERATION(glEnable(GL_BLEND))
			if (currentEffect == DefaultEffectRef) {
				currentEffect = changeShaderProgram(DefaultEffectRef, firstCall, currentColor, camera);
			}
			continue;
		} else if (rc.effectRef != currentEffect) {
			if (batchSize > 0) {
				// execute batch
				#ifdef USE_VBO
				LOGE("Error batching unsupported with VBO");
				#else
				drawBatchES2(boundTexture, vertices, uvs, indices, batchSize);
				#endif
				batchSize = 0;
			}
			currentEffect = changeShaderProgram(rc.effectRef, firstCall, currentColor, camera);
		}

		if (rc.texture != InvalidTextureRef) {
			const TextureInfo& info = textures[rc.texture];
			rc.glref = info.glref;
			Vector2 offset(rc.uv[0]);
			Vector2 scale(rc.uv[1]);
			Vector2 uvS (info.uv[1] - info.uv[0]);
			#ifdef USE_VBO
			float uvso[4];
			uvso[0] = scale.X * uvS.X;
			uvso[1] = scale.Y * uvS.Y;
			uvso[2] = info.uv[0].X + offset.X * uvS.X;
			uvso[3] = info.uv[0].Y + offset.Y * uvS.Y;
			GL_OPERATION(glUniform4fv(effectRefToShader(currentEffect, firstCall).uniformUVScaleOffset, 1, uvso))
			#else
			if (info.rotateUV) {
				std::swap(offset.X, offset.Y);
				std::swap(scale.X, scale.Y);
				offset.Y = 1 - (scale.Y + offset.Y);
			}
			{
				rc.uv[0] = info.uv[0] + Vector2(offset.X * uvS.X, offset.Y * uvS.Y);
				rc.uv[1] = rc.uv[0] + Vector2(scale.X * uvS.X, scale.Y * uvS.Y);
			}
			#endif
            if (rc.mirrorH) {
                if (info.rotateUV)
                    std::swap(rc.uv[0].Y, rc.uv[1].Y);
                else
                    std::swap(rc.uv[0].X, rc.uv[1].X);
            }
			rc.rotateUV = info.rotateUV;
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
			GL_OPERATION(glUniform4fv(effectRefToShader(currentEffect, firstCall).uniformUVScaleOffset, 1, uvso))
			#endif
		}
		t = rc.glref;
		if (boundTexture != rc.glref || currentColor != rc.color) {
			if (batchSize > 0) {
				// execute batch
				#ifdef USE_VBO
				LOGE("Error batching unsupported with VBO");
				#else
				drawBatchES2(boundTexture, vertices, uvs, indices, batchSize);
				#endif

				batchSize = 0;
			}
			boundTexture = rc.glref;
			if (currentColor != rc.color) {
	            currentColor = rc.color;
	            GL_OPERATION(glUniform4fv(effectRefToShader(currentEffect, firstCall).uniformColor, 1, currentColor.rgba))
			}
		}

		#ifdef USE_VBO
		// rc.position.X /= 2 * rc.halfSize.X;
		// rc.position.Y /= 2 * rc.halfSize.Y;
		float hW = 0.5 * screenW;
		float hH = 0.5 * screenH;
		GLfloat mat[16];
		loadOrthographicMatrix(-hW - rc.position.X + camPos.X, hW - rc.position.X + camPos.X, -hH - rc.position.Y + camPos.Y, hH - rc.position.Y + camPos.Y, 0, 1, mat);
		GL_OPERATION(glUniform1f(effectRefToShader(currentEffect, firstCall).uniformRotation, -rc.rotation))
		float scale[] = { 2 * rc.halfSize.X, 2 * rc.halfSize.Y };
		GL_OPERATION(glUniform2fv(effectRefToShader(currentEffect, firstCall).uniformScale, 1, scale))
		GL_OPERATION(glUniformMatrix4fv(effectRefToShader(currentEffect, firstCall).uniformMatrix, 1, GL_FALSE, mat))
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

		batchSize++;

		if (batchSize == MAX_BATCH_SIZE) {
				#ifdef USE_VBO
				drawBatchES2(boundTexture, rc.rotateUV);
				#else
				drawBatchES2(boundTexture, vertices, uvs, indices, batchSize);
				#endif
			batchSize = 0;
		}
	}

	if (batchSize > 0) {
		#ifdef USE_VBO
		LOGE("Error batching unsupported with VBO");
		#else
		drawBatchES2(boundTexture, vertices, uvs, indices, batchSize);
		#endif
	}
}
#include <errno.h>

void RenderingSystem::waitDrawingComplete() {
    int readQueue = (currentWriteQueue + 1) % 2;
    pthread_mutex_lock(&mutexes[L_RENDER]);
    while (renderQueue[readQueue].count > 0)
        pthread_cond_wait(&cond[C_RENDER_DONE], &mutexes[L_RENDER]);
    pthread_mutex_unlock(&mutexes[L_RENDER]);
}

void RenderingSystem::render() {
	PROFILE("Renderer", "load-textures", BeginEvent);
    processDelayedTextureJobs();
    PROFILE("Renderer", "load-textures", EndEvent);
    PROFILE("Renderer", "wait-frame", BeginEvent);
    pthread_mutex_lock(&mutexes[L_QUEUE]);
    while (!newFrameReady) {
        pthread_cond_wait(&cond[C_FRAME_READY], &mutexes[L_QUEUE]);
    }
    newFrameReady = false;
    int readQueue = (currentWriteQueue + 1) % 2;
    pthread_mutex_unlock(&mutexes[L_QUEUE]);
    PROFILE("Renderer", "wait-frame", EndEvent);
	if (pthread_mutex_trylock(&mutexes[L_RENDER]) != 0) {
		LOGW("HMM Busy render lock");
		pthread_mutex_lock(&mutexes[L_RENDER]);
	}
    PROFILE("Renderer", "render", BeginEvent);
    if (renderQueue[readQueue].count == 0) {
        LOGW("Arg, nothing to render - probably a bug");
    } else {
        RenderQueue& inQueue = renderQueue[readQueue];
        drawRenderCommands(inQueue);
        inQueue.count = 0;
    }
    pthread_cond_signal(&cond[C_RENDER_DONE]);
    pthread_mutex_unlock(&mutexes[L_RENDER]);
	PROFILE("Renderer", "render", EndEvent);
}

void computeVerticesScreenPos(const Vector2& position, const Vector2& hSize, float rotation, int rotateUV, Vector2* out) {
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

    mat[0] = 2.0f / r_l;
    mat[1] = 2.0f / t_b;
    mat[2] = -2.0f / f_n;
    mat[3] = tx;
    mat[4] = ty;
    mat[5] = tz;

    /*
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
    */
}

const RenderingSystem::Shader& RenderingSystem::effectRefToShader(EffectRef ref, bool firstCall) {
	if (ref == DefaultEffectRef) {
		return firstCall ? defaultShaderNoAlpha : defaultShader;
	} else {
		return ref2Effects[ref];
	}
}
