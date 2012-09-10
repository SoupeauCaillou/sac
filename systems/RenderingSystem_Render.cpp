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

void RenderingSystem::check_GL_errors(const char* context) {
	 int maxIterations=10;
    GLenum error;
    while (((error = glGetError()) != GL_NO_ERROR) && maxIterations > 0)
    {
        switch(error)
        {
            case GL_INVALID_ENUM:
            	LOGW("[%2d]GL error: '%s' -> GL_INVALID_ENUM\n", maxIterations, context); break;
            case GL_INVALID_VALUE:
            	LOGW("[%2d]GL error: '%s' -> GL_INVALID_VALUE\n", maxIterations, context); break;
            case GL_INVALID_OPERATION:
            	LOGW("[%2d]GL error: '%s' -> GL_INVALID_OPERATION\n", maxIterations, context); break;
            case GL_OUT_OF_MEMORY:
            	LOGW("[%2d]GL error: '%s' -> GL_OUT_OF_MEMORY\n", maxIterations, context); break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
            	LOGW("[%2d]GL error: '%s' -> GL_INVALID_FRAMEBUFFER_OPERATION\n", maxIterations, context); break;
            default:
            	LOGW("[%2d]GL error: '%s' -> %x\n", maxIterations, context, error);
        }
		  maxIterations--;
    }
}

#include <pthread.h>

#ifdef GLES2_SUPPORT
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
        LOGW("GL shader error: %s\n", log);
 		delete[] log;
    }

   if (!glIsShader(shader)) {
   	LOGW("Weird; %d is not a shader", shader);
   }
	return shader;
}
#endif

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
	GL_OPERATION(glActiveTexture(GL_TEXTURE1))
	// GL_OPERATION(glEnable(GL_TEXTURE_2D))
	if (firstCall) {
		GL_OPERATION(glBindTexture(GL_TEXTURE_2D, 0))
	} else {
		GL_OPERATION(glBindTexture(GL_TEXTURE_2D, glref.alpha))
	}
	
#ifdef USE_VBO
	GL_OPERATION(glBindBuffer(GL_ARRAY_BUFFER, theRenderingSystem.squareBuffers[reverseUV ? 1 : 0]))

	GL_OPERATION(glEnableVertexAttribArray(RenderingSystem::ATTRIB_VERTEX))
	GL_OPERATION(glEnableVertexAttribArray(RenderingSystem::ATTRIB_UV))
	GL_OPERATION(glVertexAttribPointer(RenderingSystem::ATTRIB_VERTEX, 3, GL_FLOAT, 0, 5 * sizeof(float), 0))
	GL_OPERATION(glVertexAttribPointer(RenderingSystem::ATTRIB_UV, 2, GL_FLOAT, 0, 5 * sizeof(float), (float*) 0 + 3))
	
	GL_OPERATION(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, theRenderingSystem.squareBuffers[2]))
	GL_OPERATION(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0))
#else
	GL_OPERATION(glVertexAttribPointer(RenderingSystem::ATTRIB_VERTEX, 3, GL_FLOAT, 0, 0, vertices))
	GL_OPERATION(glEnableVertexAttribArray(RenderingSystem::ATTRIB_VERTEX))
	GL_OPERATION(glVertexAttribPointer(RenderingSystem::ATTRIB_UV, 2, GL_FLOAT, 1, 0, uvs))
	GL_OPERATION(glEnableVertexAttribArray(RenderingSystem::ATTRIB_UV))

	GL_OPERATION(glDrawElements(GL_TRIANGLES, batchSize * 6, GL_UNSIGNED_SHORT, indices))
#endif
}

EffectRef RenderingSystem::changeShaderProgram(EffectRef ref, bool _firstCall, const Color& color) {
	const Shader& shader = effectRefToShader(ref, _firstCall);
	GL_OPERATION(glUseProgram(shader.program))
	GLfloat mat[16];
	loadOrthographicMatrix(-screenW*0.5, screenW*0.5, -screenH * 0.5, screenH * 0.5, 0, 1, mat);
	GL_OPERATION(glUniformMatrix4fv(shader.uniformMatrix, 1, GL_FALSE, mat))

	GL_OPERATION(glUniform1i(shader.uniformColorSampler, 0))
	GL_OPERATION(glUniform1i(shader.uniformAlphaSampler, 1))
	GL_OPERATION(glUniform4fv(shader.uniformColor, 1, color.rgba))
	return ref;
}

void RenderingSystem::drawRenderCommands(std::queue<RenderCommand>& commands, bool opengles2) {
#ifdef USE_VBO
#define MAX_BATCH_SIZE 1
#else
#define MAX_BATCH_SIZE 128
#endif
	static GLfloat vertices[MAX_BATCH_SIZE * 4 * 3];
	static GLfloat uvs[MAX_BATCH_SIZE * 4 * 2];
	static unsigned short indices[MAX_BATCH_SIZE * 6];
	int batchSize = 0;

	GL_OPERATION(glDepthMask(true))
	GL_OPERATION(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
	GL_OPERATION(glEnable(GL_DEPTH_TEST))
	GL_OPERATION(glDisable(GL_BLEND))
	InternalTexture boundTexture = InternalTexture::Invalid, t;
	Color currentColor(1,1,1,1);

	firstCall = true;
	
	EffectRef currentEffect = changeShaderProgram(DefaultEffectRef, firstCall, currentColor);
	
    while (!commands.empty()) {

		RenderCommand& rc = commands.front();

		if (rc.texture == EndFrameMarker) {
			// LOGW("Frame drawn: %u", commands.front().rotateUV);
			commands.pop();
			break;
		}
		else if (rc.texture == DisableZWriteMarker) {
			commands.pop();
			if (batchSize > 0) {
				// execute batch
				#ifdef USE_VBO
				LOGI("Error batching unsupported with VBO");
				#else
				drawBatchES2(boundTexture, vertices, uvs, indices, batchSize);
				#endif
				batchSize = 0;
			}

			firstCall = false;
			GL_OPERATION(glDepthMask(false))
			GL_OPERATION(glEnable(GL_BLEND))
			
			if (currentEffect == DefaultEffectRef) {
				currentEffect = changeShaderProgram(DefaultEffectRef, firstCall, currentColor);
			}
			continue;
		} else if (rc.effectRef != currentEffect) {
			if (batchSize > 0) {
				// execute batch
				#ifdef USE_VBO
				LOGI("Error batching unsupported with VBO");
				#else
				drawBatchES2(boundTexture, vertices, uvs, indices, batchSize);
				#endif
				batchSize = 0;
			}
			currentEffect = changeShaderProgram(rc.effectRef, firstCall, currentColor);
		}

		if (rc.texture != InvalidTextureRef) {
			const TextureInfo& info = textures[rc.texture];
			rc.glref = info.glref;
			Vector2 offset = rc.uv[0], scale = rc.uv[1];
			Vector2 uvS = info.uv[1] - info.uv[0];
			#ifdef USE_VBO
			float uvso[4];
			uvso[0] = scale.X * uvS.X;
			uvso[1] = scale.Y * uvS.Y;
			uvso[2] = info.uv[0].X + offset.X * uvS.X;
			uvso[3] = info.uv[0].Y + offset.Y * uvS.Y;
			GL_OPERATION(glUniform4fv(effectRefToShader(currentEffect, firstCall).uniformUVScaleOffset, 1, uvso))
			#else
			
			rc.uv[0] = info.uv[0] + Vector2(offset.X * uvS.X, offset.Y * uvS.Y);
			rc.uv[1] = rc.uv[0] + Vector2(scale.X * uvS.X, scale.Y * uvS.Y);
			#endif
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
				LOGI("Error batching unsupported with VBO");
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
		rc.position.X /= 2 * rc.halfSize.X;
		rc.position.Y /= 2 * rc.halfSize.Y;
		float hW = 0.5 * screenW / (2 * rc.halfSize.X);
		float hH = 0.5 * screenH / (2 * rc.halfSize.Y);
		GLfloat mat[16];
		loadOrthographicMatrix(-hW - rc.position.X, hW - rc.position.X, -hH - rc.position.Y, hH - rc.position.Y, 0, 1, mat);
		GL_OPERATION(glUniform1f(effectRefToShader(currentEffect, firstCall).uniformRotation, rc.rotation))
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
		commands.pop();
	}

	if (batchSize > 0) {
		#ifdef USE_VBO
		LOGI("Error batching unsupported with VBO");
		#else
		drawBatchES2(boundTexture, vertices, uvs, indices, batchSize);
		#endif
	}
}
#include <errno.h>

void RenderingSystem::RenderQueue::removeFrames(int count) {
	for (int i=0; i<count; i++) {
		while (commands.front().texture != EndFrameMarker)
			commands.pop();
		// LOGW("Dump frame : %u", commands.front().rotateUV);
		commands.pop();
		frameToRender--;
	}
}

void RenderingSystem::render() {
	PROFILE("Renderer", "render", BeginEvent);
	// static float begin = TimeUtil::getTime();
	// static float end = TimeUtil::getTime();

	// begin = TimeUtil::getTime();
	// LOGW("time out: %.3f", begin - end);

	// mutex locking handled in processDelayedTextureJobs
	processDelayedTextureJobs();
//LOG/W("ici1");
	#ifndef EMSCRIPTEN
	if (pthread_mutex_trylock(&mutexes) != 0) {
		// LOGW("HMM Busy render lock");
		pthread_mutex_lock(&mutexes);
	}
	int readQueue = (currentWriteQueue + 1) % 2;
	#else
//LOGW("ici2");
	int readQueue = currentWriteQueue;
	#endif
	if (renderQueue[readQueue].frameToRender == 0) {
		#ifndef EMSCRIPTEN
		readQueue = currentWriteQueue;

		if (renderQueue[readQueue].frameToRender == 0) {
			float bef = TimeUtil::getTime();

			pthread_cond_wait(&cond, &mutexes);
			LOGW("Waited : %.3f s -> %d", TimeUtil::getTime() - bef, renderQueue[readQueue].frameToRender);
		}
		currentWriteQueue = (currentWriteQueue + 1) % 2;
		#else
		LOGW("NOTHING TO RENDER");
		return;
		#endif
	} else {
#ifndef EMSCRIPTEN
		// read queue is not empty
		int rqCount = renderQueue[readQueue].frameToRender;
		int wrCount = renderQueue[currentWriteQueue].frameToRender;

		int excessFrameCount = rqCount + wrCount - 2;
		if (excessFrameCount > 0) {
			// remove half of the excess frames, to smooth the drop
			int toRemove = (int) ceil(excessFrameCount / 2.0);
			int n = MathUtil::Min(toRemove, rqCount);
			renderQueue[readQueue].removeFrames(n);

			if (n < toRemove) {
				toRemove -= n;
				renderQueue[currentWriteQueue].removeFrames(toRemove);
				readQueue = currentWriteQueue;
				currentWriteQueue = (currentWriteQueue + 1) % 2;
			} else if (renderQueue[readQueue].frameToRender == 0) {
				readQueue = currentWriteQueue;
				currentWriteQueue = (currentWriteQueue + 1) % 2;
			}
		}
#endif
	}
//LOGW("ici3 : %d", readQueue);
	// LOGW("Reading 1 frame from: %d", readQueue);
//	assert (renderQueue[readQueue].frameToRender > 0);
//	assert (readQueue != currentWriteQueue);
	RenderQueue& inQueue = renderQueue[readQueue];
	inQueue.frameToRender--;
	#ifndef EMSCRIPTEN
	pthread_mutex_unlock(&mutexes);
	#endif
// LOGW("ici4 : %d", readQueue);
	drawRenderCommands(inQueue.commands, opengles2);
// LOGW("ici5 : %d", readQueue);
	// commands.clear();
	// LOGW("redner queue size: %d OUT", renderQueue.size());

	// glFinish();
	// end = TimeUtil::getTime();
	// LOGW("time in : %.3f", end - begin);
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
}

const RenderingSystem::Shader& RenderingSystem::effectRefToShader(EffectRef ref, bool firstCall) {
	if (ref == DefaultEffectRef) {
		return firstCall ? defaultShaderNoAlpha : defaultShader;
	} else {
		return ref2Effects[ref];
	}
}
