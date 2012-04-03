#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <map>
#include <set>
#include <queue>

#include "base/Vector2.h"
#include "base/MathUtil.h"
#include "base/Log.h"

#include "System.h"
#include "TransformationSystem.h"

typedef int TextureRef;
#define InvalidTextureRef -1
#define EndFrameMarker -10
#define DisableZWriteMarker -11

#define CHECK_GL_ERROR

#ifdef CHECK_GL_ERROR
	#define GL_OPERATION(x)	\
		(x); \
		RenderingSystem::check_GL_errors(#x);
#else
	#define GL_OPERATION(x) \
		(x);
#endif

class NativeAssetLoader {
	public:
		virtual char* decompressPngImage(const std::string& assetName, int* width, int* height) = 0;

		virtual char* loadShaderFile(const std::string& assetName) = 0;
};

struct Color {
	union {
		struct {
			float rgba[4];
		};
		struct {
			float r, g, b, a;
		};
	};

	Color(float _r=1.0, float _g=1.0, float _b=1.0, float _a=1.0):
		r(_r), g(_g), b(_b), a(_a) {}
};
struct RenderingComponent {
	RenderingComponent() : /*bottomLeftUV(0, 0), topRightUV(1, 1),*/ hide(true), texture(InvalidTextureRef), drawGroup(BackToFront) {}
	// Vector2 bottomLeftUV, topRightUV;
	TextureRef texture;
	Color color;
	enum {
		BackToFront, FrontToBack
	} drawGroup;
	bool hide;
};

struct RenderCommand {
	float z;
	TextureRef texture;
	unsigned int rotateUV;
	Vector2 uv[2];
	Vector2 halfSize;
	Color color;
	Vector2 position;
	float rotation;
};

#define theRenderingSystem RenderingSystem::GetInstance()
#define RENDERING(e) theRenderingSystem.Get(e)

UPDATABLE_SYSTEM(Rendering)

public:
void init();

void loadAtlas(const std::string& atlasName);

int saveInternalState(uint8_t** out);
void restoreInternalState(const uint8_t* in, int size);

void setWindowSize(int w, int h);

TextureRef loadTextureFile(const std::string& assetName);

void setNativeAssetLoader(NativeAssetLoader* ptr) { assetLoader = ptr; }

public:
static void loadOrthographicMatrix(float left, float right, float bottom, float top, float near, float far, float* mat);
GLuint compileShader(const std::string& assetName, GLuint type);

bool isEntityVisible(Entity e);

void reloadTextures();

bool opengles2;

void render();

public:
int w,h;
/* textures cache */
TextureRef nextValidRef;
std::map<std::string, TextureRef> assetTextures;
struct TextureInfo {
	GLuint glref;
	unsigned int rotateUV;
	Vector2 uv[2];
	int atlasIndex;
	
	TextureInfo (GLuint r = 0, int x = 0, int y = 0, int w = 0, int h = 0, bool rot = false, const Vector2& size = Vector2::Zero, int atlasIdx = -1);

};
struct Atlas {
	std::string name;
	GLuint texture;
};

std::map<TextureRef, TextureInfo> textures;
std::set<std::string> delayedLoads;
std::vector<Atlas> atlas;

NativeAssetLoader* assetLoader;

int current;
int frameToRender;
std::queue<RenderCommand> renderQueue;
pthread_mutex_t mutexes[2];

/* default (and only) shader */
GLuint defaultProgram;
GLuint uniformMatrix;
GLuint whiteTexture;

/* open gl es1 var */

private:
GLuint loadTexture(const std::string& assetName, int& w, int& h);
void drawRenderCommands(std::queue<RenderCommand>& commands, bool opengles2);

public:
static void check_GL_errors(const char* context);

enum {
    ATTRIB_VERTEX = 0,
    ATTRIB_UV,
	ATTRIB_COLOR,
	ATTRIB_POS_ROT,
	ATTRIB_SCALE,
    NUM_ATTRIBS
};
};
