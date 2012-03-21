#include "RenderingSystem.h"
#include <GLES/gl.h>
#include "base/EntityManager.h"

INSTANCE_IMPL(RenderingSystem);

static void check_GL_errors(const char* context) {
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

// #define CHECK_GL_ERROR

#ifdef CHECK_GL_ERROR
	#define GL_OPERATION(x)	\
		(x); \
		check_GL_errors(#x);
#else
	#define GL_OPERATION(x) \
		(x);
#endif

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
	opengles2 = true;
}

void RenderingSystem::setWindowSize(int width, int height) {
	w = width;
	h = height;
	GL_OPERATION(glViewport(0, 0, w, h))
}

static unsigned int alignOnPowerOf2(unsigned int value) {
	for(int i=0; i<32; i++) {
		unsigned int c = 1 << i;
		if (value <= c)
			return c;
	}
	return 0;
}

RenderingSystem::TextureInfo RenderingSystem::loadTexture(const std::string& assetName) {
	int w, h;
	char* data = assetLoader->decompressPngImage(assetName, &w, &h);

	if (!data)
		return TextureInfo();

	/* create GL texture */
	if (!opengles2)
		GL_OPERATION(glEnable(GL_TEXTURE_2D))

	int powerOf2W = alignOnPowerOf2(w);
	int powerOf2H = alignOnPowerOf2(h);

	GLuint texture;
	GL_OPERATION(glGenTextures(1, &texture))
	GL_OPERATION(glBindTexture(GL_TEXTURE_2D, texture))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST))
	GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, powerOf2W,
                powerOf2H, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                NULL))
   GL_OPERATION(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE,
                data))

	TextureInfo info;
	info.glref = texture;
	info.region = Vector2(w/(float)powerOf2W, h/(float)powerOf2H);
	free(data);

	return info;
}

void RenderingSystem::reloadTextures() {
	for (std::map<std::string, TextureRef>::iterator it=assetTextures.begin(); it!=assetTextures.end(); ++it) {
		textures[it->second] = loadTexture(it->first);
	}
}

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
		return 0;
    }
	return shader;
}

void RenderingSystem::init() {
	if (opengles2) {
		defaultProgram = 0;
		LOGW("default prog: %u before", defaultProgram);
		defaultProgram = glCreateProgram();
		check_GL_errors("");
		LOGW("default prog: %u after", defaultProgram);
		
		LOGI("Compiling shaders\n");
		GLuint vs = compileShader("default.vs", GL_VERTEX_SHADER);
		GLuint fs = compileShader("default.fs", GL_FRAGMENT_SHADER);

		GL_OPERATION(glAttachShader(defaultProgram, vs))
		GL_OPERATION(glAttachShader(defaultProgram, fs))
		LOGI("Binding GLSL attribs\n");
		GL_OPERATION(glBindAttribLocation(defaultProgram, ATTRIB_VERTEX, "aPosition"))
		GL_OPERATION(glBindAttribLocation(defaultProgram, ATTRIB_UV, "aTexCoord"))
		GL_OPERATION(glBindAttribLocation(defaultProgram, ATTRIB_COLOR, "aColor"))
		GL_OPERATION(glBindAttribLocation(defaultProgram, ATTRIB_POS_ROT, "aPosRot"))

		LOGI("Linking GLSL program\n");
		GL_OPERATION(glLinkProgram(defaultProgram))

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
	} else {
		float ratio = h / (float)w ;
		GL_OPERATION(glEnable(GL_TEXTURE_2D))
		GL_OPERATION(glClearColor(0.2, 0.5, 0.1, 1.0))
	
	#if ANDROID
		GL_OPERATION(glEnable(GL_TEXTURE_2D))
		GL_OPERATION(glMatrixMode(GL_PROJECTION))
		GL_OPERATION(glLoadIdentity())
		GL_OPERATION(glOrthof(-5., 5.0f, -5. * ratio, 5. * ratio, 0, 1))
		GL_OPERATION(glMatrixMode(GL_MODELVIEW))
		GL_OPERATION(glLoadIdentity())
		GL_OPERATION(glEnableClientState(GL_VERTEX_ARRAY))
		GL_OPERATION(glEnableClientState(GL_TEXTURE_COORD_ARRAY))
	#endif
	}

	// create 1px white texture
	uint8_t data[] = {255, 255, 255, 255};
	GL_OPERATION(glGenTextures(1, &whiteTexture))
	GL_OPERATION(glBindTexture(GL_TEXTURE_2D, whiteTexture))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST))
	GL_OPERATION(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST))
	GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1,
                1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                data))
                
	GL_OPERATION(glEnable(GL_BLEND))
	GL_OPERATION(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA))
	GL_OPERATION(glEnable(GL_DEPTH_TEST))
	GL_OPERATION(glDepthFunc(GL_GEQUAL))
	GL_OPERATION(glClearDepthf(0.0))
	// GL_OPERATION(glDepthRangef(0, 1))
}

TextureRef RenderingSystem::loadTextureFile(const std::string& assetName) {
	TextureRef result = InvalidTextureRef;

	if (assetTextures.find(assetName) != assetTextures.end()) {
		result = assetTextures[assetName];
	} else {
		result = nextValidRef++;
		assetTextures[assetName] = result;
	}

	if (textures.find(result) != textures.end())
		return result;

	textures[result] = loadTexture(assetName);

	return result;
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

bool sortFrontToBack(const RenderCommand& r1, const RenderCommand& r2) {
	if (r1.z == r2.z)
		return r1.texture < r2.texture;
	else
		return r1.z > r2.z;
}

void RenderingSystem::DoUpdate(float dt) {
	GL_OPERATION(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT))

	
	if (opengles2) {
		float ratio = h / (float)w ;
		GL_OPERATION(glUseProgram(defaultProgram))
		GLfloat mat[16];
		loadOrthographicMatrix(-5., 5.0f, -5. * ratio, 5. * ratio, 0, 1, mat);
		GL_OPERATION(glUniformMatrix4fv(uniformMatrix, 1, GL_FALSE, mat))
	}

	std::vector<RenderCommand> commands, semiOpaqueCommands;
	
	/* render */
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;
		RenderingComponent* rc = (*it).second;
		
		if (rc->hide || rc->color.a <= 0) {
			continue;
		}

		const TransformationComponent* tc = TRANSFORM(a);

		RenderCommand c;
		c.uv[0] = rc->bottomLeftUV;
		c.uv[1] = rc->topRightUV;
		if (rc->texture != InvalidTextureRef) {
			TextureInfo info = textures[rc->texture];
			c.texture = info.glref;
			c.uv[0].X *= info.region.X;
			c.uv[1].X *= info.region.X;
			c.uv[0].Y *= info.region.Y;
			c.uv[1].Y *= info.region.Y;
		} else {
			c.texture = whiteTexture;
		}
		c.halfSize = tc->size * 0.5f;
		c.color = rc->color;
		c.position = tc->worldPosition;
		c.rotation = tc->worldRotation;
		c.z = tc->z;

		if (true || c.color.a >= 1)
			commands.push_back(c);
		else
			semiOpaqueCommands.push_back(c);
	}

	GLuint boundTexture = 0;
	std::sort(commands.begin(), commands.end(), sortFrontToBack);
	for(std::vector<RenderCommand>::iterator it=commands.begin(); it!=commands.end(); it++) {
		const RenderCommand& rc = *it;

		if (boundTexture != rc.texture) {
			GL_OPERATION(glBindTexture(GL_TEXTURE_2D, rc.texture))
			boundTexture = rc.texture;
		}
		const GLfloat squareVertices[] = {
				-rc.halfSize.X, -rc.halfSize.Y,
				rc.halfSize.X, -rc.halfSize.Y,
				-rc.halfSize.X, rc.halfSize.Y,
				rc.halfSize.X, rc.halfSize.Y
							};

		const GLfloat squareUvs[] = {
			rc.uv[0].X, rc.uv[0].Y,
			rc.uv[1].X, rc.uv[0].Y,
			rc.uv[0].X, rc.uv[1].Y,
			rc.uv[1].X, rc.uv[1].Y
		};
		float col[16];
		for(int i=0; i<4; i++)
			memcpy(&col[4*i], rc.color.rgba, 4 * sizeof(float));
		float posRot[] = {
			rc.position.X, rc.position.Y, rc.z, rc.rotation,
			rc.position.X, rc.position.Y, rc.z, rc.rotation,
			rc.position.X, rc.position.Y, rc.z, rc.rotation,
			rc.position.X, rc.position.Y, rc.z, rc.rotation
		};
		if (opengles2) {
			GL_OPERATION(glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, 0, 0, squareVertices))
			GL_OPERATION(glEnableVertexAttribArray(ATTRIB_VERTEX))
			GL_OPERATION(glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, 1, 0, squareUvs))
			GL_OPERATION(glEnableVertexAttribArray(ATTRIB_UV))
			GL_OPERATION(glVertexAttribPointer(ATTRIB_COLOR, 4, GL_FLOAT, 1, 0, col))
			GL_OPERATION(glEnableVertexAttribArray(ATTRIB_COLOR))
			GL_OPERATION(glVertexAttribPointer(ATTRIB_POS_ROT, 4, GL_FLOAT, 0, 0, posRot))
			GL_OPERATION(glEnableVertexAttribArray(ATTRIB_POS_ROT))
			GL_OPERATION(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4))
		} else {
			GL_OPERATION(glPushMatrix())
			GL_OPERATION(glTranslatef(rc.position.X, rc.position.Y, rc.z))
			#define PI 3.14159265f
			GL_OPERATION(glRotatef(180 * rc.rotation / PI , 0, 0, 1))
			GL_OPERATION(glScalef(rc.halfSize.X * 2, rc.halfSize.Y * 2, 1.0f))
			GL_OPERATION(glColor4f(rc.color.r, rc.color.g, rc.color.b, rc.color.a))
			float vertexBuffer[] = {
				 -0.5f,  -0.5f,
				 0.5f,   -0.5f,
				 -0.5f,  0.5f,
				 0.5f,  0.5f,
			};
			GL_OPERATION(glVertexPointer(2, GL_FLOAT, 0, vertexBuffer))
			GL_OPERATION(glTexCoordPointer(2, GL_FLOAT, 0, squareUvs))
			GL_OPERATION(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4))
			GL_OPERATION(glPopMatrix())
		}
	}
}

bool RenderingSystem::isEntityVisible(Entity e) {
	const Vector2 halfSize = TRANSFORM(e)->size * 0.5;
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


int RenderingSystem::saveInternalState(uint8_t** out) {
	int size = 0;
	for (std::map<std::string, TextureRef>::iterator it=assetTextures.begin(); it!=assetTextures.end(); ++it) {
		size += (*it).first.length() + 1;
		size += sizeof(TextureRef);
	}

	*out = new uint8_t[size];
	uint8_t* ptr = *out;
	for (std::map<std::string, TextureRef>::iterator it=assetTextures.begin(); it!=assetTextures.end(); ++it) {
		ptr = (uint8_t*) mempcpy(ptr, (*it).first.c_str(), (*it).first.length() + 1);
		ptr = (uint8_t*) mempcpy(ptr, &(*it).second, sizeof(TextureRef));
	}
	return size;
}

void RenderingSystem::restoreInternalState(const uint8_t* in, int size) {
	assetTextures.clear();
	nextValidRef = 1;

	int idx = 0;
	while (idx < size) {
		char name[128];
		int i=0;
		do {
			name[i] = in[idx++];
		} while (name[i++] != '\0');
		TextureRef ref;
		memcpy(&ref, &in[idx], sizeof(TextureRef));
		idx += sizeof(TextureRef);

		assetTextures[name] = ref;
		nextValidRef = MathUtil::Max(nextValidRef, ref + 1);
	}
}
