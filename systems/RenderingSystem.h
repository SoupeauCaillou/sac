/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#ifdef ANDROID
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#else
#include <GL/glew.h>
#endif
#include <map>
#include <set>
#include <queue>

#include "base/Vector2.h"
#include "base/MathUtil.h"
#include "base/Log.h"
#include "../api/AssetAPI.h"

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
#define GLES2_SUPPORT

struct Color {
	union {
		struct {
			float rgba[4];
		};
		struct {
			float r, g, b, a;
		};
	};

 	static Color random() {
 		return (Color( MathUtil::RandomFloat(), MathUtil::RandomFloat(), MathUtil::RandomFloat()));
 	}

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
	RenderingComponent() : texture(InvalidTextureRef), color(Color()), hide(true), desaturate(false), opaqueType(NON_OPAQUE) {}
	
	TextureRef texture;
	Color color;
	bool hide, desaturate;
	enum Opacity {
		NON_OPAQUE = 0,
		FULL_OPAQUE,
		OPAQUE_ABOVE,
		OPAQUE_UNDER
	} ;
	Opacity opaqueType;
	float opaqueSeparation; // €[0, 1], meaning depends of opaqueType
};

#define theRenderingSystem RenderingSystem::GetInstance()
#define RENDERING(e) theRenderingSystem.Get(e)

UPDATABLE_SYSTEM(Rendering)

public:
void init();

void loadAtlas(const std::string& atlasName, bool forceImmediateTextureLoading = false);
void unloadAtlas(const std::string& atlasName);
void invalidateAtlasTextures();

int saveInternalState(uint8_t** out);
void restoreInternalState(const uint8_t* in, int size);

void setWindowSize(int w, int h, float sW, float sH);

TextureRef loadTextureFile(const std::string& assetName);

void unloadTexture(TextureRef ref, bool allowUnloadAtlas = false);

public:
AssetAPI* assetAPI;

static void loadOrthographicMatrix(float left, float right, float bottom, float top, float near, float far, float* mat);
GLuint compileShader(const std::string& assetName, GLuint type);

bool isEntityVisible(Entity e);
bool isVisible(TransformationComponent* tc);

void reloadTextures();

bool opengles2;

void render();

public:
int windowW, windowH;
float screenW, screenH;
/* textures cache */
TextureRef nextValidRef;
std::map<std::string, TextureRef> assetTextures;

struct InternalTexture {
	GLuint color;
	GLuint alpha;
	
	/*void operator=(GLuint r) {
		color = alpha = r;
	}*/
	bool operator==(const InternalTexture& t) const {
		return color == t.color && alpha == t.alpha;
	}
	bool operator!=(const InternalTexture& t) const {
		return color != t.color || alpha != t.alpha;
	}
	bool operator<(const InternalTexture& t) const {
		return color < t.color;
	}
	
	static InternalTexture Invalid; 
};

struct RenderCommand {
	float z;
	union {
		TextureRef texture;
		InternalTexture glref;
	};
	unsigned int rotateUV;
	Vector2 uv[2];
	Vector2 halfSize;
	Color color;
	Vector2 position;
	float rotation;
	bool desaturate;
};

struct RenderQueue {
	RenderQueue() : frameToRender(0) {}
	int frameToRender;
	std::queue<RenderCommand> commands;
	
	void removeFrames(int count);
};

struct TextureInfo {
	InternalTexture glref;
	unsigned int rotateUV;
	Vector2 uv[2];
	int atlasIndex;
	
	TextureInfo (const InternalTexture& glref = InternalTexture::Invalid, int x = 0, int y = 0, int w = 0, int h = 0, bool rot = false, const Vector2& size = Vector2::Zero, int atlasIdx = -1);
};
struct Atlas {
	std::string name;
	InternalTexture glref;
};

std::map<TextureRef, TextureInfo> textures;
std::set<std::string> delayedLoads;
std::set<int> delayedAtlasIndexLoad;
std::set<InternalTexture> delayedDeletes;
std::vector<Atlas> atlas;

int currentWriteQueue;
RenderQueue renderQueue[2]; // 
pthread_mutex_t mutexes;
pthread_cond_t cond;

/* default (and only) shader */
struct Shader {
	GLuint program;
	GLuint uniformMatrix, uniformColorSampler, uniformAlphaSampler, uniformColor;
};

Shader defaultShader, defaultShaderNoAlpha, desaturateShader;
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
void loadTexture(const std::string& assetName, Vector2& realSize, Vector2& pow2Size, InternalTexture& out);
void drawRenderCommands(std::queue<RenderCommand>& commands, bool opengles2);
void processDelayedTextureJobs();
GLuint createGLTexture(const std::string& basename, bool colorOrAlpha, Vector2& realSize, Vector2& pow2Size);
public:
static void check_GL_errors(const char* context);
Shader buildShader(const std::string& vs, const std::string& fs);
void changeShaderProgram(const Shader& shader, const Color& color);
enum {
    ATTRIB_VERTEX = 0,
    ATTRIB_UV,
	ATTRIB_POS_ROT,
	ATTRIB_SCALE,
    NUM_ATTRIBS
};
};
