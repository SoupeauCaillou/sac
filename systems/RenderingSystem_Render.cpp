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
#ifndef ANDROID
#include <GL/glew.h>
#else
#include <GLES/gl.h>
#endif
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

#ifdef GLES2_SUPPORT
bool firstCall;
static void drawBatchES2(const RenderingSystem::InternalTexture& glref, const GLfloat* vertices, const GLfloat* uvs, const unsigned short* indices, int batchSize) {
	GL_OPERATION(glActiveTexture(GL_TEXTURE0))
	// GL_OPERATION(glEnable(GL_TEXTURE_2D))
	GL_OPERATION(glBindTexture(GL_TEXTURE_2D, glref.color))
	GL_OPERATION(glActiveTexture(GL_TEXTURE1))
	// GL_OPERATION(glEnable(GL_TEXTURE_2D))
	if (firstCall) {
		GL_OPERATION(glBindTexture(GL_TEXTURE_2D, 0))
	} else {
		GL_OPERATION(glBindTexture(GL_TEXTURE_2D, glref.alpha))
	}
	
	GL_OPERATION(glVertexAttribPointer(RenderingSystem::ATTRIB_VERTEX, 3, GL_FLOAT, 0, 0, vertices))
	GL_OPERATION(glEnableVertexAttribArray(RenderingSystem::ATTRIB_VERTEX))
	GL_OPERATION(glVertexAttribPointer(RenderingSystem::ATTRIB_UV, 2, GL_FLOAT, 1, 0, uvs))
	GL_OPERATION(glEnableVertexAttribArray(RenderingSystem::ATTRIB_UV))
	// GL_OPERATION(glVertexAttribPointer(ATTRIB_POS_ROT, 4, GL_FLOAT, 0, 0, posrot))
	// GL_OPERATION(glEnableVertexAttribArray(ATTRIB_POS_ROT))
	//GL_OPERATION(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4))
	GL_OPERATION(glDrawElements(GL_TRIANGLES, batchSize * 6, GL_UNSIGNED_SHORT, indices))
}
#endif

static void setupTexturing(const RenderingSystem::InternalTexture& glref, bool enableDesaturation, const float* uvs) {
	if (!enableDesaturation) {
		//// # TEXTURE STAGE 1
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, glref.color);
		glClientActiveTexture(GL_TEXTURE0);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		// color: modulate TEXTURE color with glColor
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		// alpha: use glColor value
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		
		GL_OPERATION(glTexCoordPointer(2, GL_FLOAT, 0, uvs))
		GL_OPERATION(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
		
		//// # TEXTURE STAGE 2
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, glref.alpha);
		glClientActiveTexture(GL_TEXTURE1);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		// use previous stage color
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		// combine previous alpha with TEXTURE alpha
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
		
		GL_OPERATION(glTexCoordPointer(2, GL_FLOAT, 0, uvs))
		GL_OPERATION(glEnableClientState(GL_TEXTURE_COORD_ARRAY))

		//// # TEXTURE STAGE 3 (disabled)
		glActiveTexture(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);
	} else {
		//Enable texture unit 0 to divide RGB values in our texture by 2
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, glref.color);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glClientActiveTexture(GL_TEXTURE0);
		
		//GL_MODULATE is Arg0 * Arg1    
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		
		//Configure Arg1
		float multipliers[4] = {.5, .5, .5, 1.0};
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, (GLfloat*)&multipliers);
		
		//Remember to set your texture coordinates if you need them
		GL_OPERATION(glTexCoordPointer(2, GL_FLOAT, 0, uvs))
			GL_OPERATION(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
		
		//Enable texture unit 1 to increase RGB values by .5
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, glref.alpha);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glClientActiveTexture(GL_TEXTURE1);
		
		//GL_ADD is Arg0 + Arg1
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
		
		// combine previous alpha with TEXTURE alpha
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
		
		//Configure Arg0
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		
		//Configure Arg1
		GLfloat additions[4] = {.5, .5, .5, 0.0};
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, (GLfloat*)&additions);
		
		//Set your texture coordinates if you need them
		GL_OPERATION(glTexCoordPointer(2, GL_FLOAT, 0, uvs))
			GL_OPERATION(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
		
		//Enable texture combiner 2 to get a DOT3_RGB product of your RGB values
		glActiveTexture(GL_TEXTURE2);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, glref.color);
		glClientActiveTexture(GL_TEXTURE2);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		
		//GL_DOT3_RGB is 4*((Arg0r - 0.5) * (Arg1r - 0.5) + (Arg0g - 0.5) * (Arg1g - 0.5) + (Arg0b - 0.5) * (Arg1b - 0.5))
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_DOT3_RGB);   
		
		//Configure Arg0
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		
		//Configure Arg1
		//We want this to adjust our DOT3 by R*0.3 + G*0.59 + B*0.11
		//So, our actual adjustment will need to take into consideration
		//the fact that OpenGL will subtract .5 from our Arg1
		//and we need to also take into consideration that we have divided 
		//our RGB values by 2 and we are multiplying the entire
		//DOT3 product by 4
		//So, for Red adjustment you will get :
		//   .65 = (4*(0.3))/2 + 0.5  = (0.3/2) + 0.5
		GLfloat weights[4] = {.65, .795, .555, 1.};
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, (GLfloat*)&weights);
		
		GL_OPERATION(glTexCoordPointer(2, GL_FLOAT, 0, uvs))
		GL_OPERATION(glEnableClientState(GL_TEXTURE_COORD_ARRAY))		
	}
}

static bool desaturate = false;
static void drawBatchES1(const RenderingSystem::InternalTexture& glref, const GLfloat* vertices, const GLfloat* uvs, const unsigned short* indices, int batchSize) {
	if (glref == RenderingSystem::InternalTexture::Invalid)
		return;
	setupTexturing(glref, desaturate, uvs);
	
	GL_OPERATION(glVertexPointer(2, GL_FLOAT, 0, vertices))
	
	// GL_OPERATION(glColorPointer(4, GL_FLOAT, 0, colors))
	

	GL_OPERATION(glDrawElements(GL_TRIANGLES, batchSize * 6, GL_UNSIGNED_SHORT, indices))
	
	GL_OPERATION(glEnable(GL_BLEND))
}

#ifdef GLES2_SUPPORT
void RenderingSystem::changeShaderProgram(const Shader& shader, const Color& color) {
	GL_OPERATION(glUseProgram(shader.program))
	GLfloat mat[16];
	loadOrthographicMatrix(-screenW*0.5, screenW*0.5, -screenH * 0.5, screenH * 0.5, 0, 1, mat);
	GL_OPERATION(glUniformMatrix4fv(shader.uniformMatrix, 1, GL_FALSE, mat))
		
	GL_OPERATION(glUniform1i(shader.uniformColorSampler, 0))
	GL_OPERATION(glUniform1i(shader.uniformAlphaSampler, 1))
	GL_OPERATION(glUniform4fv(shader.uniformColor, 1, color.rgba))
}
#endif

void RenderingSystem::drawRenderCommands(std::queue<RenderCommand>& commands, bool opengles2) {
#define MAX_BATCH_SIZE 128
	static GLfloat vertices[MAX_BATCH_SIZE * 4 * 3];
	static GLfloat uvs[MAX_BATCH_SIZE * 4 * 2];
	static unsigned short indices[MAX_BATCH_SIZE * 6];
	int batchSize = 0;
	desaturate = false;
	
	GL_OPERATION(glDepthMask(true))
	GL_OPERATION(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))
	GL_OPERATION(glEnable(GL_DEPTH_TEST))
	GL_OPERATION(glDisable(GL_BLEND))
	InternalTexture boundTexture = InternalTexture::Invalid, t;
	Color currentColor(1,1,1,1);
	
	if (!opengles2) {
		GL_OPERATION(glEnableClientState(GL_VERTEX_ARRAY))
		GL_OPERATION(glDisableClientState(GL_COLOR_ARRAY))
    	glColor4f(currentColor.r, currentColor.g, currentColor.b, currentColor.a);
	} else {
		firstCall = true;
    	changeShaderProgram(defaultShaderNoAlpha, currentColor);
	}
   
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
				#ifdef GLES2_SUPPORT
				if (opengles2)
					drawBatchES2(boundTexture, vertices, uvs, indices, batchSize);
				else
				#endif
					drawBatchES1(boundTexture, vertices, uvs, indices, batchSize);

				batchSize = 0;
			}
			
			firstCall = false;
			GL_OPERATION(glDepthMask(false))
			GL_OPERATION(glEnable(GL_BLEND))
			if (!desaturate) {
				changeShaderProgram(defaultShader, currentColor);
			}
			continue;
		} else if (rc.desaturate != desaturate) {
			if (batchSize > 0) {
				// execute batch
				#ifdef GLES2_SUPPORT
				if (opengles2)
					drawBatchES2(boundTexture, vertices, uvs, indices, batchSize);
				else
				#endif
					drawBatchES1(boundTexture, vertices, uvs, indices, batchSize);

				batchSize = 0;
			}
			desaturate = !desaturate;
			#ifdef GLES2_SUPPORT
			if (opengles2) {
				changeShaderProgram(desaturate ? desaturateShader : (firstCall ? defaultShaderNoAlpha : defaultShader), currentColor);
			}
			#endif
		}

		if (rc.texture != InvalidTextureRef) {
			const TextureInfo& info = textures[rc.texture];
			rc.glref = info.glref;
			Vector2 offset = rc.uv[0], scale = rc.uv[1];
			Vector2 uvS = info.uv[1] - info.uv[0];
			rc.uv[0] = info.uv[0] + Vector2(offset.X * uvS.X, offset.Y * uvS.Y);
			rc.uv[1] = rc.uv[0] + Vector2(scale.X * uvS.X, scale.Y * uvS.Y);
			rc.rotateUV = info.rotateUV;
		} else {
			rc.glref = InternalTexture::Invalid;
			rc.glref.color = whiteTexture;
			rc.glref.alpha = whiteTexture;
			rc.uv[0] = Vector2::Zero;
			rc.uv[1] = Vector2(1,1);
			rc.rotateUV = 0;
		}
		t = rc.glref;
		if (boundTexture != rc.glref || currentColor != rc.color) {
			if (batchSize > 0) {
				// execute batch
				#ifdef GLES2_SUPPORT
				if (opengles2)
					drawBatchES2(boundTexture, vertices, uvs, indices, batchSize);
				else
				#endif
					drawBatchES1(boundTexture, vertices, uvs, indices, batchSize);

				batchSize = 0;
			}
			boundTexture = rc.glref;
			
			if (currentColor != rc.color) {
	            currentColor = rc.color;
	            if (!opengles2) {
	            	glColor4f(currentColor.r, currentColor.g, currentColor.b, currentColor.a);
	            } else {
	            	if (desaturate) {
	            		GL_OPERATION(glUniform4fv(desaturateShader.uniformColor, 1, currentColor.rgba))
	            	} else if (firstCall) {
		            	GL_OPERATION(glUniform4fv(defaultShaderNoAlpha.uniformColor, 1, currentColor.rgba))
	            	} else {
		            	GL_OPERATION(glUniform4fv(defaultShader.uniformColor, 1, currentColor.rgba))
	            	}
	            }
			}
		}

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

		batchSize++;

		if (batchSize == MAX_BATCH_SIZE) {
			#ifdef GLES2_SUPPORT
			if (opengles2)
				drawBatchES2(rc.glref, vertices, uvs, indices, batchSize);
			else
			#endif
				drawBatchES1(rc.glref, vertices, uvs, indices, batchSize);
			batchSize = 0;
		}
		commands.pop();
	}

	if (batchSize > 0) {
		#ifdef GLES2_SUPPORT
		if (opengles2) {
			drawBatchES2(t, vertices, uvs, indices, batchSize);
		} else 
		#endif	
		{
			drawBatchES1(t, vertices, uvs, indices, batchSize);
		}
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
	// static float begin = TimeUtil::getTime();
	// static float end = TimeUtil::getTime();
	
	// begin = TimeUtil::getTime();
	// LOGW("time out: %.3f", begin - end);
	
	// mutex locking handled in processDelayedTextureJobs
	processDelayedTextureJobs();
	
	if (pthread_mutex_trylock(&mutexes) != 0) {
		// LOGW("HMM Busy render lock");
		pthread_mutex_lock(&mutexes);
	}

	int readQueue = (currentWriteQueue + 1) % 2;
	if (renderQueue[readQueue].frameToRender == 0) {
		readQueue = currentWriteQueue;
		
		if (renderQueue[readQueue].frameToRender == 0) {
			float bef = TimeUtil::getTime();
			pthread_cond_wait(&cond, &mutexes);
			LOGW("Waited : %.3f s -> %d", TimeUtil::getTime() - bef, renderQueue[readQueue].frameToRender);
		}
		currentWriteQueue = (currentWriteQueue + 1) % 2;
	} else {
		// read queue is not empty
		int rqCount = renderQueue[readQueue].frameToRender;
		int wrCount = renderQueue[currentWriteQueue].frameToRender;
		
		int toRemove = rqCount + wrCount - 2;
		if (toRemove > 0) {
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
	}
	// LOGW("Reading 1 frame from: %d", readQueue);
	assert (renderQueue[readQueue].frameToRender > 0);
	assert (readQueue != currentWriteQueue);
	RenderQueue& inQueue = renderQueue[readQueue];
	inQueue.frameToRender--;
	pthread_mutex_unlock(&mutexes);

	drawRenderCommands(inQueue.commands, opengles2);
	// commands.clear();
	// LOGW("redner queue size: %d OUT", renderQueue.size());

	// glFinish();
	// end = TimeUtil::getTime();
	// LOGW("time in : %.3f", end - begin);
}

void computeVerticesScreenPos(const Vector2& position, const Vector2& hSize, float rotation, int rotateUV, Vector2* out) {
	const float cr = cos(rotation);
	const float sr = sin(rotation);

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

