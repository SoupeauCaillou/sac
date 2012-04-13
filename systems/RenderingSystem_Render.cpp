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
   	LOGW("Weird; %d is not a shader");
   }
	return shader;
}

static void computeVerticesScreenPos(const Vector2& position, const Vector2& hSize, float rotation, int rotateUV, Vector2* out);

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

static void drawBatchES1(const GLfloat* vertices, const GLfloat* uvs, const GLfloat* colors, const GLfloat* posrot, const unsigned short* indices, int batchSize) {
	GL_OPERATION(glVertexPointer(3, GL_FLOAT, 0, vertices))
	GL_OPERATION(glEnableClientState(GL_VERTEX_ARRAY))
	GL_OPERATION(glTexCoordPointer(2, GL_FLOAT, 0, uvs))
	GL_OPERATION(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
	GL_OPERATION(glColorPointer(4, GL_FLOAT, 0, colors))
	GL_OPERATION(glEnableClientState(GL_COLOR_ARRAY))

	GL_OPERATION(glDrawElements(GL_TRIANGLES, batchSize * 6, GL_UNSIGNED_SHORT, indices))
}

void RenderingSystem::drawRenderCommands(std::queue<RenderCommand>& commands, bool opengles2) {
#define MAX_BATCH_SIZE 64
	static GLfloat vertices[MAX_BATCH_SIZE * 4 * 3];
	static GLfloat uvs[MAX_BATCH_SIZE * 4 * 2];
	static GLfloat colors[MAX_BATCH_SIZE * 4 * 4];
	static GLfloat posrot[MAX_BATCH_SIZE * 4 * 4];
	static unsigned short indices[MAX_BATCH_SIZE * 6];
	int batchSize = 0;

	// GL_OPERATION(glDepthMask(true))
	GLuint boundTexture = 0;
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
		}
		
		if (rc.texture != InvalidTextureRef) {
			TextureInfo info = textures[rc.texture];
			rc.texture = info.glref;
			rc.uv[0] = info.uv[0];
			rc.uv[1] = info.uv[1];
			rc.rotateUV = info.rotateUV;
		} else {
			rc.uv[0] = Vector2::Zero;
			rc.uv[1] = Vector2(1,1);
			rc.rotateUV = 0;
		}

		if (boundTexture != rc.texture) {
			if (batchSize > 0) {
				// execute batch
				if (opengles2)
					drawBatchES2(vertices, uvs, colors, posrot, indices, batchSize);
				else
					drawBatchES1(vertices, uvs, colors, posrot, indices, batchSize);
				
				batchSize = 0;
			}
			GL_OPERATION(glBindTexture(GL_TEXTURE_2D, rc.texture))
			boundTexture = rc.texture;
		}

		const GLfloat squareUvs[] = {
			rc.uv[0].X, rc.uv[0].Y,
			rc.uv[1].X, rc.uv[0].Y,
			rc.uv[0].X, rc.uv[1].Y,
			rc.uv[1].X, rc.uv[1].Y
		};
		
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
			
		memcpy(&colors[baseIdx * 4 ], rc.color.rgba, 4 * sizeof(float));
		memcpy(&colors[(baseIdx + 1) * 4], rc.color.rgba, 4 * sizeof(float));
		memcpy(&colors[(baseIdx + 2) * 4], rc.color.rgba, 4 * sizeof(float));
		memcpy(&colors[(baseIdx + 3) * 4], rc.color.rgba, 4 * sizeof(float));

		indices[batchSize * 6 + 0] = baseIdx + 0;
		indices[batchSize * 6 + 1] = baseIdx + 1;
		indices[batchSize * 6 + 2] = baseIdx + 2;
		indices[batchSize * 6 + 3] = baseIdx + 1;
		indices[batchSize * 6 + 4] = baseIdx + 3;
		indices[batchSize * 6 + 5] = baseIdx + 2;
			
		batchSize++;
			
		if (batchSize == MAX_BATCH_SIZE) {
			if (opengles2)
				drawBatchES2(vertices, uvs, colors, posrot, indices, batchSize);
			else
				drawBatchES1(vertices, uvs, colors, posrot, indices, batchSize);
			batchSize = 0;
		}
		commands.pop();
	}
	
	if (batchSize > 0)
		if (opengles2)
			drawBatchES2(vertices, uvs, colors, posrot, indices, batchSize);
		else
			drawBatchES1(vertices, uvs, colors, posrot, indices, batchSize);
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

	if (opengles2) {
		float ratio = h / (float)w ;
		GL_OPERATION(glUseProgram(defaultProgram))
		GLfloat mat[16];
		loadOrthographicMatrix(-5., 5.0f, -5. * ratio, 5. * ratio, 0, 1, mat);
		GL_OPERATION(glUniformMatrix4fv(uniformMatrix, 1, GL_FALSE, mat))
	}

	for (std::set<std::string>::iterator it=delayedLoads.begin(); it != delayedLoads.end(); ++it) {
		LOGI("Delayed loading of: %s", (*it).c_str());
		int w,h;
		textures[assetTextures[*it]] = loadTexture(*it, w, h);
	}
	delayedLoads.clear();
	
	GL_OPERATION(glClear(GL_COLOR_BUFFER_BIT /*| GL_DEPTH_BUFFER_BIT*/))

	
	// std::vector<RenderCommand>& commands = renderCommands[cmd];	
	drawRenderCommands(renderQueue /*commands*/, opengles2);
	// commands.clear();
	// LOGW("redner queue size: %d OUT", renderQueue.size());	
	frameToRender--;
end:
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

