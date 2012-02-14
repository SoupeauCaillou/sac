#pragma once

#include "System.h"
#include "../base/Vector2.h"
#include <GLES2/gl2.h>
#include <map>

typedef int TextureRef;

typedef char* (*DecompressPNGImagePtr) (const char* assetName, int* width, int* height);
typedef char* (*LoadShaderPtr) (const char* assetName);

struct RenderingComponent {
	RenderingComponent() : size(1.0f, 1.0f), bottomLeftUV(0, 0), topRightUV(1, 1), hide(false) {}
	Vector2 size;
	Vector2 bottomLeftUV, topRightUV;
	TextureRef texture;
	bool hide;
};

#define theRenderingSystem RenderingSystem::GetInstance()
#define RENDERING(e) theRenderingSystem.Get(e)

UPDATABLE_SYSTEM(Rendering)

public:
void init();

void setWindowSize(int w, int h);

TextureRef loadTextureFile(const std::string& assetName);

void setDecompressPNGImagePtr(DecompressPNGImagePtr ptr) { decompressPNG = ptr; }
void setLoadShaderPtr(LoadShaderPtr ptr) { loadShaderPtr = ptr; }

public:
static void loadOrthographicMatrix(float left, float right, float bottom, float top, float near, float far, float* mat);
GLuint compileShader(const std::string& assetName, GLuint type);

private:
int w,h;
/* textures cache */
TextureRef nextValidRef;
std::map<std::string, TextureRef> assetTextures;
std::map<TextureRef, GLuint> textures;

DecompressPNGImagePtr decompressPNG;
LoadShaderPtr loadShaderPtr;

/* default (and only) shader */ 
GLuint defaultProgram;
GLuint uniformMatrix;
};
