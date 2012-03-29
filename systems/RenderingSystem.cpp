#include "RenderingSystem.h"
#include <GLES/gl.h>
#include "base/EntityManager.h"
#include <cmath>
#include <cassert>
#include <sstream>

RenderingSystem::TextureInfo::TextureInfo (GLuint r, int x, int y, int w, int h, bool rot, const Vector2& size) {
	glref = r;		
	if (size == Vector2::Zero) {
		uv[0].X = uv[0].Y = 0;
		uv[1].X = uv[1].Y = 1;
		rotateUV = 0;
	} else {
		float blX = x / size.X;
		float trX = (x+w) / size.X;
		float blY = 1 - (y+h) / size.Y;
		float trY = 1 - y / size.Y;

		uv[0].X = blX;
		uv[1].X = trX;
		uv[0].Y = blY;
		uv[1].Y = trY;
		rotateUV = rot;
	}
} 
	
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

#define CHECK_GL_ERROR

#ifdef CHECK_GL_ERROR
	#define GL_OPERATION(x)	\
		(x); \
		check_GL_errors(#x);
#else
	#define GL_OPERATION(x) \
		(x);
#endif

#include <pthread.h>

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
	current = 0;
	frameToRender = 0;
	pthread_mutex_init(&mutexes[0], 0);
	pthread_mutex_init(&mutexes[1], 0);
}

void RenderingSystem::setWindowSize(int width, int height) {
	w = width;
	h = height;
	GL_OPERATION(glViewport(0, 0, w, h))
}

#include <fstream>

static void parse(const std::string& line, std::string& assetName, int& x, int& y, int& w, int& h, bool& rot) {
	std::string substrings[6];
	int from = 0, to = 0;
	for (int i=0; i<6; i++) {
		to = line.find_first_of(',', from);
		substrings[i] = line.substr(from, to - from);
		from = to + 1;
	}
	assetName = substrings[0];
	x = atoi(substrings[1].c_str());
	y = atoi(substrings[2].c_str());
	w = atoi(substrings[3].c_str());
	h = atoi(substrings[4].c_str());
	rot = atoi(substrings[5].c_str());
}

void RenderingSystem::loadAtlas(const std::string& atlasName) {
	std::string atlasDesc = atlasName + ".desc";
	std::string atlasImage = atlasName + ".png";
	
	const char* desc = assetLoader->loadShaderFile(atlasDesc);
	if (!desc) {
		LOGW("Unable to load atlas desc %s", atlasDesc.c_str());
		return;
	}
	
	int w, h;
	GLuint glref = loadTexture(atlasImage, w,h );
	Vector2 atlasSize(w, h);
	
	std::stringstream f(std::string(desc), std::ios_base::in);
	std::string s;
	f >> s;
	while(!s.empty()) {
		std::cout << "line:'" << s << "'" << std::endl;
		std::string assetName;
		int x, y, w, h;
		bool rot;

		parse(s, assetName, x, y, w, h, rot);

		TextureRef result = nextValidRef++;
		assetTextures[assetName] = result;
		textures[result] = TextureInfo(glref, x, y, w, h, rot, atlasSize);
		
		s.clear();
		f >> s;
	}
}

static unsigned int alignOnPowerOf2(unsigned int value) {
	for(int i=0; i<32; i++) {
		unsigned int c = 1 << i;
		if (value <= c)
			return c;
	}
	return 0;
}

GLuint RenderingSystem::loadTexture(const std::string& assetName, int& w, int& h) {
	char* data = assetLoader->decompressPngImage(assetName, &w, &h);

	if (!data)
		return 0;

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
	GL_OPERATION(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w,
                h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data))
	free(data);
	return texture;
}

void RenderingSystem::reloadTextures() {
	int w, h;
	for (std::map<std::string, TextureRef>::iterator it=assetTextures.begin(); it!=assetTextures.end(); ++it) {
		textures[it->second] = loadTexture(it->first, w, h);
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
    }
    
   if (!glIsShader(shader)) {
   	LOGW("Weird; %d is not a shader");
   }
	return shader;
}

void RenderingSystem::init() {
	if (opengles2) {
		defaultProgram = 0;
		LOGW("default prog: %u before", defaultProgram);
		defaultProgram = glCreateProgram();
		check_GL_errors("glCreateProgram");
		LOGW("default prog: %u after [isAProgram = %d]", defaultProgram, glIsProgram(defaultProgram));
		
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
		     LOGW("GL shader program error: '%s'", log);
		     delete[] log;
		 }

		uniformMatrix = glGetUniformLocation(defaultProgram, "uMvp");

		glDeleteShader(vs);
		glDeleteShader(fs);
	} else {
		float ratio = h / (float)w ;
		GL_OPERATION(glEnable(GL_TEXTURE_2D))
		GL_OPERATION(glClearColor(0.2, 0.5, 0.1, 1.0))
		glAlphaFunc(GL_GREATER, 0.1);
		glEnable(GL_ALPHA_TEST);
	
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
	// GL_OPERATION(glEnable(GL_DEPTH_TEST))
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
	if (textures.find(result) == textures.end())
		delayedLoads.insert(assetName);
		
	return result;
}

static bool sortFrontToBack(const RenderCommand& r1, const RenderCommand& r2) {
	if (r1.z == r2.z)
		return r1.texture < r2.texture;
	else
		return r1.z > r2.z;
}

static bool sortBackToFront(const RenderCommand& r1, const RenderCommand& r2) {
	if (r1.z == r2.z)
		return r1.texture < r2.texture;
	else
		return r1.z < r2.z;
}

static void computeVerticesScreenPos(const Vector2& position, const Vector2& hSize, float rotation, int rotateUV, Vector2* out);

static void drawBatchES2(const GLfloat* vertices, const GLfloat* uvs, const GLfloat* colors, const GLfloat* posrot, const unsigned short* indices, int batchSize) {
	GL_OPERATION(glVertexAttribPointer(ATTRIB_VERTEX, 3, GL_FLOAT, 0, 0, vertices))
	GL_OPERATION(glEnableVertexAttribArray(ATTRIB_VERTEX))
	GL_OPERATION(glVertexAttribPointer(ATTRIB_UV, 2, GL_FLOAT, 1, 0, uvs))
	GL_OPERATION(glEnableVertexAttribArray(ATTRIB_UV))
	GL_OPERATION(glVertexAttribPointer(ATTRIB_COLOR, 4, GL_FLOAT, 1, 0, colors))
	GL_OPERATION(glEnableVertexAttribArray(ATTRIB_COLOR))
	// GL_OPERATION(glVertexAttribPointer(ATTRIB_POS_ROT, 4, GL_FLOAT, 0, 0, posrot))
	// GL_OPERATION(glEnableVertexAttribArray(ATTRIB_POS_ROT))
	//GL_OPERATION(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4))

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

	GL_OPERATION(glDepthMask(true))
	GLuint boundTexture = 0;
	while (!commands.empty()) {
	// for(std::vector<RenderCommand>::iterator it=commands.begin(); it!=commands.end(); it++) {
		RenderCommand& rc = commands.front();
		commands.pop();
		
		if (rc.texture == EndFrameMarker)
			break;
		else if (rc.texture == DisableZWriteMarker)
			GL_OPERATION(glDepthMask(false))
		
		if (rc.texture != InvalidTextureRef) {
			TextureInfo info = textures[rc.texture];
			rc.texture = info.glref;
			rc.uv[0] = info.uv[0];
			rc.uv[1] = info.uv[1];
			rc.rotateUV = info.rotateUV;
		} else {
			rc.texture = whiteTexture;
			rc.uv[0] = Vector2::Zero;
			rc.uv[1] = Vector2(1,1);
			rc.rotateUV = 0;
		}

		if (boundTexture != rc.texture) {
			if (opengles2 && batchSize > 0) {
				// execute batch
				drawBatchES2(vertices, uvs, colors, posrot, indices, batchSize);
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
		
		if (opengles2) {
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
				drawBatchES2(vertices, uvs, colors, posrot, indices, batchSize);
				batchSize = 0;
			}
		} else {
		#if 1
			GL_OPERATION(glPushMatrix())
			GL_OPERATION(glTranslatef(rc.position.X, rc.position.Y, -rc.z))
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
		#endif
		}
	}
	
	if (batchSize > 0)
		drawBatchES2(vertices, uvs, colors, posrot, indices, batchSize);
		
}

void RenderingSystem::DoUpdate(float dt) {
	pthread_mutex_lock(&mutexes[current]);
	
	std::vector<RenderCommand> commands;
	std::vector<RenderCommand> semiOpaqueCommands;
	
	/* render */
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;
		RenderingComponent* rc = (*it).second;
		
		if (rc->hide || rc->color.a <= 0) {
			continue;
		}

		const TransformationComponent* tc = TRANSFORM(a);

		RenderCommand c;
		c.texture = rc->texture;
		c.halfSize = tc->size * 0.5f;
		c.color = rc->color;
		c.position = tc->worldPosition;
		c.rotation = tc->worldRotation;
		c.z = tc->z;

		if (c.color.a >= 1 && rc->drawGroup == RenderingComponent::FrontToBack)
			commands.push_back(c);
		else
			semiOpaqueCommands.push_back(c);
	}

	std::sort(commands.begin(), commands.end(), sortFrontToBack);
	std::sort(semiOpaqueCommands.begin(), semiOpaqueCommands.end(), sortBackToFront);
	
	for(std::vector<RenderCommand>::iterator it=commands.begin(); it!=commands.end(); it++) {
		renderQueue.push(*it);
	}
	RenderCommand dummy;
	dummy.texture = DisableZWriteMarker;
	renderQueue.push(dummy);
	for(std::vector<RenderCommand>::iterator it=semiOpaqueCommands.begin(); it!=semiOpaqueCommands.end(); it++) {
		renderQueue.push(*it);
	}
	dummy.texture = EndFrameMarker;
	renderQueue.push(dummy);
	frameToRender++;
	// LOGW("Added: %d + %d + 1 elt (%d frames)", commands.size(), semiOpaqueCommands.size(), frameToRender);
	
	pthread_mutex_unlock(&mutexes[current]);
	
	//current = (current + 1) % 2;
}

void RenderingSystem::render() {
	pthread_mutex_lock(&mutexes[current]);
	// LOGW("redner queue size: %d IN - frame count: %d", renderQueue.size(), frameToRender);	
	if (renderQueue.empty() || frameToRender == 0)
		goto end;
	
	// drop the late fram
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
