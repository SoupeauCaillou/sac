#include "RenderingSystem.h"


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

void RenderingSystem::reset() {
	assetTextures.clear();
}

GLuint RenderingSystem::compileShader(const std::string& assetName, GLuint type) {
	char* source = assetLoader->loadShaderFile(assetName);
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, (const char**)&source, NULL);
	glCompileShader(shader);

	delete[] source;
  	GLint logLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 1)
    {
        char *log = new char[logLength];
        glGetShaderInfoLog(shader, logLength, &logLength, log);
        LOGW("GL shader error: %s\n", log);
 		delete[] log;
		return 0;
    }
	return shader;
}

void RenderingSystem::init() {
	reset();

	LOGI("Compiling shaders\n");
	GLuint vs = compileShader("default.vs", GL_VERTEX_SHADER);
	GLuint fs = compileShader("default.fs", GL_FRAGMENT_SHADER);

	defaultProgram = glCreateProgram();
	glAttachShader(defaultProgram, vs);
	glAttachShader(defaultProgram, fs);
	LOGI("Binding GLSL attribs\n");
	glBindAttribLocation(defaultProgram, ATTRIB_VERTEX, "aPosition");
    glBindAttribLocation(defaultProgram, ATTRIB_UV, "aTexCoord");
	glBindAttribLocation(defaultProgram, ATTRIB_COLOR, "aColor");
	glBindAttribLocation(defaultProgram, ATTRIB_POS_ROT, "aPosRot");

	LOGI("Linking GLSL program\n");
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

	if (!assetLoader)
		return 0;

	int w, h;
	char* data = assetLoader->decompressPngImage(assetName, &w, &h);

	if (!data)
		return 0;

	/* create GL texture */
	//glEnable(GL_TEXTURE_2D);

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

struct RenderCommand {
	float z;
	TextureRef texture;
	Vector2 halfSize;
	Vector2 uv[2];
	Color color;
	Vector2 position;
	float rotation;
};

bool sortRender(const RenderCommand& r1, const RenderCommand& r2) {
	if (r1.z == r2.z)
		return r1.texture < r2.texture;
	else
		return r1.z < r2.z;
}

void RenderingSystem::DoUpdate(float dt) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);

	glUseProgram(defaultProgram);
	float ratio = h / (float)w ;
	GLfloat mat[16];
	loadOrthographicMatrix(-5., 5.0f, -5. * ratio, 5. * ratio, 0, 1, mat);
	glUniformMatrix4fv(uniformMatrix, 1, GL_FALSE, mat);

	std::vector<RenderCommand> commands;

	/* render */
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;
		RenderingComponent* rc = (*it).second;

		if (rc->hide) {
			//LOGI("entity %d hidden\n", a);
			continue;
		}

		const TransformationComponent* tc = TRANSFORM(a);

		if (rc->texture > 0)
		{

		}
		else {
			//LOGI("entity %d has no texture\n", a);
			continue;
		}

		RenderCommand c;
		c.texture = textures[rc->texture];
		c.halfSize = rc->size * 0.5f;
		c.uv[0] = rc->bottomLeftUV;
		c.uv[1] = rc->topRightUV;
		c.color = rc->color;
		c.position = tc->worldPosition;
		c.rotation = tc->worldRotation;
		c.z = tc->z;

		commands.push_back(c);
	}

	std::sort(commands.begin(), commands.end(), sortRender);

	for(std::vector<RenderCommand>::iterator it=commands.begin(); it!=commands.end(); it++) {
		const RenderCommand& rc = *it;

		glBindTexture(GL_TEXTURE_2D, rc.texture);

		const GLfloat squareVertices[] = {
				-rc.halfSize.X, -rc.halfSize.Y, 0.,
				rc.halfSize.X, -rc.halfSize.Y,0.,
				-rc.halfSize.X, rc.halfSize.Y,0.,
				rc.halfSize.X, rc.halfSize.Y,0.
			};

		const GLfloat squareUvs[] = {
			rc.uv[0].X, rc.uv[0].Y,
			rc.uv[1].X,rc.uv[0].Y,
			rc.uv[0].X, rc.uv[1].Y,
			rc.uv[1].X, rc.uv[1].Y
		};

		glVertexAttribPointer(ATTRIB_VERTEX, 3, GL_FLOAT, 0, 0, squareVertices);
		glEnableVertexAttribArray(ATTRIB_VERTEX);
		glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, 1, 0, squareUvs);
		glEnableVertexAttribArray(ATTRIB_UV);
		float col[16];
		for(int i=0; i<4; i++)
			memcpy(&col[4*i], rc.color.rgba, 4 * sizeof(float));
		glVertexAttribPointer(ATTRIB_COLOR, 4, GL_FLOAT, 1, 0, col);
		glEnableVertexAttribArray(ATTRIB_COLOR);
		float posRot[] = {
			rc.position.X, rc.position.Y, 0, rc.rotation,
			rc.position.X, rc.position.Y, 0, rc.rotation,
			rc.position.X, rc.position.Y, 0, rc.rotation,
			rc.position.X, rc.position.Y, 0, rc.rotation
		 };
		 //LOGI("[%d %d] tex:%d {%.2f %.2f} {%.2f %.2f}\n", w, h, rc.texture, rc.position.X, rc.position.Y, rc.halfSize.X, rc.halfSize.Y);
		glVertexAttribPointer(ATTRIB_POS_ROT, 4, GL_FLOAT, 0, 0, posRot);
		glEnableVertexAttribArray(ATTRIB_POS_ROT);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	}
}

bool RenderingSystem::isEntityVisible(Entity e) {
	const Vector2 halfSize = RENDERING(e)->size * 0.5;
	const Vector2& pos = TRANSFORM(e)->worldPosition;
	const float ratio = h / (float)w ;

	if (pos.X + halfSize.X < -5)
		return false;
	if (pos.X - halfSize.X > 5)
		return false;
	if (pos.Y + halfSize.Y < -5 * ratio)
		return false;
	if (pos.Y - halfSize.Y > 5 * ratio)
		return false;
	return true;
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
