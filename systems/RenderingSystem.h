#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#ifdef ANDROID
#include <GLES2/gl2.h>
#else
#include <GL/glew.h>
#endif
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
#define ToggleDesaturationMarker -12

#ifndef ANDROID
#define CHECK_GL_ERROR
#endif

#ifdef CHECK_GL_ERROR
	#define GL_OPERATION(x)	\
		(x); \
		RenderingSystem::check_GL_errors(#x);
#else
	#define GL_OPERATION(x) \
		(x);
#endif

#undef GLES2_SUPPORT


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

     Color operator*(float s) const {
        return Color(r*s, g*s, b*s, a*s);
     }
     Color operator+(const Color& c) const {
        return Color(r+c.r, g+c.g, b+c.b, a+c.a);
     }

     bool operator!=(const Color& c) const {
        return memcmp(rgba, c.rgba, sizeof(rgba)) != 0;
     }
     bool operator<(const Color& c) const {
        return memcmp(rgba, c.rgba, sizeof(rgba)) < 0;
     }

};
struct RenderingComponent {
	RenderingComponent() : texture(InvalidTextureRef), color(Color()), drawGroup(BackToFront), hide(true), desaturate(false) {}
	
	TextureRef texture;
	Color color;
	enum {
		BackToFront, FrontToBack
	} drawGroup;
	bool hide, desaturate;
};

struct RenderCommand {
	float z;
	union {
		TextureRef texture;
		GLuint glref[2];
	};
	unsigned int rotateUV;
	Vector2 uv[2];
	Vector2 halfSize;
	Color color;
	Vector2 position;
	float rotation;
	bool desaturate;
};

#define theRenderingSystem RenderingSystem::GetInstance()
#define RENDERING(e) theRenderingSystem.Get(e)

UPDATABLE_SYSTEM(Rendering)

public:
void init();

void loadAtlas(const std::string& atlasName);
void unloadAtlas(const std::string& atlasName);
void invalidateAtlasTextures();

int saveInternalState(uint8_t** out);
void restoreInternalState(const uint8_t* in, int size);

void setWindowSize(int w, int h, float sW, float sH);

TextureRef loadTextureFile(const std::string& assetName);

void unloadTexture(TextureRef ref, bool allowUnloadAtlas = false);

void setNativeAssetLoader(NativeAssetLoader* ptr) { assetLoader = ptr; }

public:
static void loadOrthographicMatrix(float left, float right, float bottom, float top, float near, float far, float* mat);
GLuint compileShader(const std::string& assetName, GLuint type);

bool isEntityVisible(Entity e);
bool isVisible(TransformationComponent* tc);

void reloadTextures();

bool opengles2;

void render();

public:
int w,h;
float screenW, screenH;
/* textures cache */
TextureRef nextValidRef;
std::map<std::string, TextureRef> assetTextures;
struct TextureInfo {
	GLuint glref[2];
	unsigned int rotateUV;
	Vector2 uv[2];
	int atlasIndex;
	
	TextureInfo (GLuint color = 0, GLuint alpha = 0, int x = 0, int y = 0, int w = 0, int h = 0, bool rot = false, const Vector2& size = Vector2::Zero, int atlasIdx = -1);

};
struct Atlas {
	std::string name;
	GLuint texture[2];
};

std::map<TextureRef, TextureInfo> textures;
std::set<std::string> delayedLoads;
std::set<int> delayedAtlasIndexLoad;
std::set<TextureRef> delayedDeletes;
std::vector<Atlas> atlas;

NativeAssetLoader* assetLoader;

int current;
int frameToRender;
std::queue<RenderCommand> renderQueue;
pthread_mutex_t mutexes[2];
pthread_cond_t cond;

/* default (and only) shader */
GLuint defaultProgram;
GLuint uniformMatrix;
GLuint whiteTexture;

/* open gl es1 var */

#ifndef ANDROID
int inotifyFd;
struct NotifyInfo {
    int wd;
    std::string asset;
};
std::vector<NotifyInfo> notifyList;
#endif

private:
void loadTexture(const std::string& assetName, Vector2& realSize, Vector2& pow2Size, GLuint* out);
void drawRenderCommands(std::queue<RenderCommand>& commands, bool opengles2);
void processDelayedTextureJobs();

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
