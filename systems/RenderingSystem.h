#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <map>

#include "base/Vector2.h"
#include "base/MathUtil.h"
#include "base/Log.h"

#include "System.h"
#include "TransformationSystem.h"

typedef int TextureRef;
#define InvalidTextureRef -1

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
	RenderingComponent() : bottomLeftUV(0, 0), topRightUV(1, 1), hide(true), texture(InvalidTextureRef) {}
	Vector2 bottomLeftUV, topRightUV;
	TextureRef texture;
	Color color;
	bool hide;
};

#define theRenderingSystem RenderingSystem::GetInstance()
#define RENDERING(e) theRenderingSystem.Get(e)

UPDATABLE_SYSTEM(Rendering)

public:
void init();

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

private:
GLuint loadTexture(const std::string& assetName);

public:
int w,h;
/* textures cache */
TextureRef nextValidRef;
std::map<std::string, TextureRef> assetTextures;
std::map<TextureRef, GLuint> textures;

NativeAssetLoader* assetLoader;

/* default (and only) shader */
GLuint defaultProgram;
GLuint uniformMatrix;
GLuint whiteTexture;

/* open gl es1 var */

};
