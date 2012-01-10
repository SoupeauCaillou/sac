#pragma once

#include "System.h"
#include "../base/Vector2.h"
#include <GL/gl.h>
#include <map>

typedef int TextureRef;

typedef char* (*DecompressPNGImagePtr) (const char* assetName, int* width, int* height);

struct RenderingComponent {
	RenderingComponent() : size(1.0f, 1.0f) {}
	Vector2 size;
	TextureRef texture;
};

#define theRenderingSystem RenderingSystem::GetInstance()
#define RENDERING(e) theRenderingSystem.Get(e)

UPDATABLE_SYSTEM(Rendering)

public:
void setWindowSize(int w, int h);

TextureRef loadTextureFile(const std::string& assetName);

void setDecompressPNGImagePtr(DecompressPNGImagePtr ptr) { decompressPNG = ptr; }

private:
TextureRef nextValidRef;
std::map<TextureRef, GLuint> textures;
DecompressPNGImagePtr decompressPNG;
};
