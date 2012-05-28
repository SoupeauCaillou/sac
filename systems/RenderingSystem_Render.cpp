#include "RenderingSystem.h"
#include <GLES/gl.h>
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
	char* source = assetLoader->loadShaderFile(assetName);
	GLuint shader = glCreateShader(type);
	GL_OPERATION(glShaderSource(shader, 1, (const char**)&source, NULL))
	GL_OPERATION(glCompileShader(shader))

	delete[] source;
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
static void drawBatchES2(const GLfloat* vertices, const GLfloat* uvs, const GLfloat* colors, const GLfloat* posrot, const unsigned short* indices, int batchSize) {
	GL_OPERATION(glVertexAttribPointer(RenderingSystem::ATTRIB_VERTEX, 3, GL_FLOAT, 0, 0, vertices))
	GL_OPERATION(glEnableVertexAttribArray(RenderingSystem::ATTRIB_VERTEX))
	GL_OPERATION(glVertexAttribPointer(RenderingSystem::ATTRIB_UV, 2, GL_FLOAT, 1, 0, uvs))
	GL_OPERATION(glEnableVertexAttribArray(RenderingSystem::ATTRIB_UV))
	GL_OPERATION(glVertexAttribPointer(RenderingSystem::ATTRIB_COLOR, 4, GL_FLOAT, 1, 0, colors))
	GL_OPERATION(glEnableVertexAttribArray(RenderingSystem::ATTRIB_COLOR))
	// GL_OPERATION(glVertexAttribPointer(ATTRIB_POS_ROT, 4, GL_FLOAT, 0, 0, posrot))
	// GL_OPERATION(glEnableVertexAttribArray(ATTRIB_POS_ROT))
	//GL_OPERATION(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4))
	GL_OPERATION(glDrawElements(GL_TRIANGLES, batchSize * 6, GL_UNSIGNED_SHORT, indices))
}
#endif

static void setupTexturing(GLint m_textureId, bool enableDesaturation, const float* uvs) {
	if (!enableDesaturation) {
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		glClientActiveTexture(GL_TEXTURE0);
		GL_OPERATION(glTexCoordPointer(2, GL_FLOAT, 0, uvs))
		GL_OPERATION(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		glActiveTexture(GL_TEXTURE1);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE2);
		glDisable(GL_TEXTURE_2D);
	} else {
		//Enable texture unit 0 to divide RGB values in our texture by 2
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glClientActiveTexture(GL_TEXTURE0);
		
		//GL_MODULATE is Arg0 * Arg1    
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
		
		//Configure Arg0
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
		
		//Configure Arg1
		float multipliers[4] = {.5, .5, .5, 0.0};
		glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, (GLfloat*)&multipliers);
		
		//Remember to set your texture coordinates if you need them
		GL_OPERATION(glTexCoordPointer(2, GL_FLOAT, 0, uvs))
			GL_OPERATION(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
		
		//Enable texture unit 1 to increase RGB values by .5
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glClientActiveTexture(GL_TEXTURE1);
		
		//GL_ADD is Arg0 + Arg1
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
		
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
		glBindTexture(GL_TEXTURE_2D, m_textureId);
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
static void drawBatchES1(GLint m_textureId, const GLfloat* vertices, const GLfloat* uvs, const unsigned short* indices, int batchSize) {
	setupTexturing(m_textureId, desaturate, uvs);
	GL_OPERATION(glVertexPointer(3, GL_FLOAT, 0, vertices))
	GL_OPERATION(glEnableClientState(GL_VERTEX_ARRAY))
	// GL_OPERATION(glColorPointer(4, GL_FLOAT, 0, colors))
	GL_OPERATION(glDisableClientState(GL_COLOR_ARRAY))

	GL_OPERATION(glDrawElements(GL_TRIANGLES, batchSize * 6, GL_UNSIGNED_SHORT, indices))
}

void RenderingSystem::drawRenderCommands(std::queue<RenderCommand>& commands, bool opengles2) {
#define MAX_BATCH_SIZE 64
	static GLfloat vertices[MAX_BATCH_SIZE * 4 * 3];
	static GLfloat uvs[MAX_BATCH_SIZE * 4 * 2];
#ifdef GLES2_SUPPORT
	static GLfloat posrot[MAX_BATCH_SIZE * 4 * 4];
#endif
	static unsigned short indices[MAX_BATCH_SIZE * 6];
	int batchSize = 0;
	desaturate = false;
	GLint t;
	// GL_OPERATION(glDepthMask(true))
	GLuint boundTexture = 0;
    Color currentColor(1,1,1,1);
    glColor4f(currentColor.r, currentColor.g, currentColor.b, currentColor.a);
    while (!commands.empty()) {
	// for(std::vector<RenderCommand>::iterator it=commands.begin(); it!=commands.end(); it++) {
		RenderCommand& rc = commands.front();

		if (rc.texture == EndFrameMarker) {
			commands.pop();
			break;
		}
		else if (rc.texture == DisableZWriteMarker) {
			commands.pop();
			continue;
		} else if (rc.desaturate != desaturate) {
			if (batchSize > 0) {
				// execute batch
				#ifdef GLES2_SUPPORT
				if (opengles2)
					drawBatchES2(vertices, uvs, 0, posrot, indices, batchSize);
				else
				#endif
					drawBatchES1(boundTexture, vertices, uvs, indices, batchSize);

				batchSize = 0;
			}
			desaturate = !desaturate;
		}

		if (rc.texture != InvalidTextureRef) {
			TextureInfo info = textures[rc.texture];
			memcpy(rc.glref, info.glref, sizeof(rc.glref));
			rc.uv[0] = info.uv[0];
			rc.uv[1] = info.uv[1];
			rc.rotateUV = info.rotateUV;
		} else {
			rc.texture = whiteTexture;
			rc.uv[0] = Vector2::Zero;
			rc.uv[1] = Vector2(1,1);
			rc.rotateUV = 0;
		}
		t = rc.glref[0];
		if (boundTexture != rc.glref[0] || currentColor != rc.color) {
			if (batchSize > 0) {
				// execute batch
				#ifdef GLES2_SUPPORT
				if (opengles2)
					drawBatchES2(vertices, uvs, 0, posrot, indices, batchSize);
				else
				#endif
					drawBatchES1(boundTexture, vertices, uvs, indices, batchSize);

				batchSize = 0;
			}
			boundTexture = rc.glref[0];
            currentColor = rc.color;
            glColor4f(currentColor.r, currentColor.g, currentColor.b, currentColor.a);
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
		uvs[baseIdx * 2 + 1] = rc.uv[0].Y;
		uvs[baseIdx * 2 + 2] = rc.uv[1].X;
		uvs[baseIdx * 2 + 3] = rc.uv[0].Y;
		uvs[baseIdx * 2 + 4] = rc.uv[0].X;
		uvs[baseIdx * 2 + 5] = rc.uv[1].Y;
		uvs[baseIdx * 2 + 6] = rc.uv[1].X;
		uvs[baseIdx * 2 + 7] = rc.uv[1].Y;

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
				drawBatchES2(vertices, uvs, 0, posrot, indices, batchSize);
			else
			#endif
				drawBatchES1(rc.glref[0], vertices, uvs, indices, batchSize);
			batchSize = 0;
		}
		commands.pop();
	}

	if (batchSize > 0) {
		#ifdef GLES2_SUPPORT
		if (opengles2) {
			drawBatchES2(vertices, uvs, 0, posrot, indices, batchSize);
		} else 
		#endif	
		{
			drawBatchES1(t, vertices, uvs, indices, batchSize);
		}
	}
}

void RenderingSystem::render() {
	pthread_mutex_lock(&mutexes[current]);
	// LOGW("redner queue size: %d IN - frame count: %d", renderQueue.size(), frameToRender);
	if (renderQueue.empty() || frameToRender == 0) {
		pthread_cond_wait(&cond, &mutexes[current]);
		// LOGW("DAMNED nothing to render %d", frameToRender);
	}

	// drop the late frames
	while (frameToRender > 1) {
		while (renderQueue.front().texture != EndFrameMarker)
			renderQueue.pop();
		renderQueue.pop();
		frameToRender--;
		// LOGW("\t %d left / %d frames", renderQueue.size(), frameToRender);
	}

#ifdef GLES2_SUPPORT
	if (opengles2) {
		GL_OPERATION(glUseProgram(defaultProgram))
		GLfloat mat[16];
		loadOrthographicMatrix(-screenW*0.5, screenW*0.5, -screenH * 0.5, screenH * 0.5, 0, 1, mat);
		GL_OPERATION(glUniformMatrix4fv(uniformMatrix, 1, GL_FALSE, mat))
	}
#endif

    processDelayedTextureJobs();

	GL_OPERATION(glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/))


	// std::vector<RenderCommand>& commands = renderCommands[cmd];
	drawRenderCommands(renderQueue /*commands*/, opengles2);
	// commands.clear();
	// LOGW("redner queue size: %d OUT", renderQueue.size());
	frameToRender--;

	pthread_mutex_unlock(&mutexes[0]);
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

