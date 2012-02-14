#include "RenderingSystem.h"
#include "../base/MathUtil.h"

#include <cstdio>
#include <cstdlib>

#include "TransformationSystem.h"

INSTANCE_IMPL(RenderingSystem);

enum {
    ATTRIB_VERTEX = 0,
    ATTRIB_UV,
	ATTRIB_COLOR,
	ATTRIB_POS_ROT,
	ATTRIB_SCALE,
    NUM_ATTRIBS
};
	
RenderingSystem::RenderingSystem() : ComponentSystemImpl<RenderingComponent>("rendering") { 
	nextValidRef = 1;
}

void RenderingSystem::setWindowSize(int width, int height) {
	w = width;
	h = height;
	glViewport(0, 0, w, h);	
}

GLuint RenderingSystem::compileShader(const std::string& assetName, GLuint type) {
	char* source = (*loadShaderPtr)(assetName.c_str());
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, (const char**)&source, NULL);
	glCompileShader(shader);

	free(source);
  	GLint logLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 1)
    {
        char *log = new char[logLength];
        glGetShaderInfoLog(shader, logLength, &logLength, log);
        std::cout << "GL shader error: " << log << std::endl;
 		delete[] log;
		return 0;
    }
	return shader;
}

void RenderingSystem::init() {
	GLuint vs = compileShader("default.vs", GL_VERTEX_SHADER);
	GLuint fs = compileShader("default.fs", GL_FRAGMENT_SHADER);

	defaultProgram = glCreateProgram();
	glAttachShader(defaultProgram, vs);
	glAttachShader(defaultProgram, fs);

	glBindAttribLocation(defaultProgram, ATTRIB_VERTEX, "aPosition");
    glBindAttribLocation(defaultProgram, ATTRIB_UV, "aTexCoord");
	glBindAttribLocation(defaultProgram, ATTRIB_COLOR, "aColor");
	glBindAttribLocation(defaultProgram, ATTRIB_POS_ROT, "aPosRot");

	glLinkProgram(defaultProgram);
 
    GLint logLength;
 	glGetProgramiv(defaultProgram, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 1)
    {
        char *log = new char[logLength];
 		glGetProgramInfoLog(defaultProgram, logLength, &logLength, log);
        std::cout << "GL shader program error: " << log << std::endl;
        delete[] log;
    }

	uniformMatrix = glGetUniformLocation(defaultProgram, "uMvp");

	glDeleteShader(vs);
	glDeleteShader(fs);
}

TextureRef RenderingSystem::loadTextureFile(const std::string& assetName) {
	if (assetTextures.find(assetName) != assetTextures.end())
		return assetTextures[assetName];

	if (!decompressPNG)
		return 0;

	int w, h;
	char* data = (*decompressPNG)(assetName.c_str(), &w, &h);

	if (!data)
		return 0;

	/* create GL texture */
	glEnable(GL_TEXTURE_2D);

	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w,
                h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                data);

	textures[nextValidRef] = texture;
	assetTextures[assetName] = nextValidRef;

	free(data);

	return nextValidRef++;
}

void RenderingSystem::DoUpdate(float dt) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glUseProgram(defaultProgram);
	float ratio = h / (float)w ;
	GLfloat mat[16];
	loadOrthographicMatrix(-5., 5.0f, -5. * ratio, 5. * ratio, 0, 1, mat);
	glUniformMatrix4fv(uniformMatrix, 1, GL_FALSE, mat);

	/* render */
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;			
		RenderingComponent* rc = (*it).second;
		const TransformationComponent* tc = TRANSFORM(a);

		if (rc->texture > 0)
		{
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textures[rc->texture]);
		}
		else {
			continue;
		}

		Vector2 hSize(rc->size * 0.5f);
		const GLfloat squareVertices[] = {
				-hSize.X, -hSize.Y, 0.,
				hSize.X, -hSize.Y,0.,
				-hSize.X, hSize.Y,0.,
				hSize.X, hSize.Y,0.
			};

		const GLfloat squareUvs[] = {
			rc->bottomLeftUV.X, rc->bottomLeftUV.Y,
			rc->topRightUV.X,rc->bottomLeftUV.Y,
			rc->bottomLeftUV.X, rc->topRightUV.Y,
			rc->topRightUV.X, rc->topRightUV.Y
		};

		glVertexAttribPointer(ATTRIB_VERTEX, 3, GL_FLOAT, 0, 0, squareVertices);
		glEnableVertexAttribArray(ATTRIB_VERTEX);
		glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, 1, 0, squareUvs);
		glEnableVertexAttribArray(ATTRIB_UV);
		float posRot[] = { 
			tc->worldPosition.X, tc->worldPosition.Y, 0.0, tc->worldRotation,
			tc->worldPosition.X, tc->worldPosition.Y, 0.0, tc->worldRotation,
			tc->worldPosition.X, tc->worldPosition.Y, 0.0, tc->worldRotation,
			tc->worldPosition.X, tc->worldPosition.Y, 0.0, tc->worldRotation
		 };
		glVertexAttribPointer(ATTRIB_POS_ROT, 4, GL_FLOAT, 0, 0, posRot);
		glEnableVertexAttribArray(ATTRIB_POS_ROT);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
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

