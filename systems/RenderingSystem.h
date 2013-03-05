#pragma once

#include <cstdio>
#include <cstdlib>
#include <algorithm>

#include "opengl/OpenglHelper.h"
#include "opengl/TextureLibrary.h"
#include <set>
#include <queue>
#include <list>

#include "base/Vector2.h"
#include "base/Color.h"
#include "../api/AssetAPI.h"

#include "System.h"

typedef int TextureRef;
#define InvalidTextureRef -1

#ifdef EMSCRIPTEN
#define USE_VBO
#endif

typedef int EffectRef;
#define DefaultEffectRef -1

typedef int FramebufferRef;
#define DefaultFrameBufferRef -1

class TransformationComponent;

struct RenderingComponent {
	RenderingComponent() : texture(InvalidTextureRef), effectRef(DefaultEffectRef), color(Color()), hide(true), mirrorH(false), zPrePass(false), fastCulling(false), opaqueType(NON_OPAQUE), cameraBitMask(0x1) {
        fbo = false;
    }

    union {
	    TextureRef texture;
        FramebufferRef framebuffer;
    };
	EffectRef effectRef;
	Color color;
	bool hide, mirrorH, zPrePass, fastCulling, fbo;
	enum Opacity {
		NON_OPAQUE = 0,
		FULL_OPAQUE
	} ;
	Opacity opaqueType;
    unsigned cameraBitMask;
};

#define theRenderingSystem RenderingSystem::GetInstance()
#define RENDERING(e) theRenderingSystem.Get(e)

UPDATABLE_SYSTEM(Rendering)

public:
~RenderingSystem();
void init();

void loadAtlas(const std::string& atlasName, bool forceImmediateTextureLoading = false);
void unloadAtlas(const std::string& atlasName);
void invalidateAtlasTextures();

int saveInternalState(uint8_t** out);
void restoreInternalState(const uint8_t* in, int size);

void setWindowSize(int w, int h, float sW, float sH);

TextureRef loadTextureFile(const std::string& assetName);
EffectRef loadEffectFile(const std::string& assetName);
FramebufferRef createFramebuffer(const std::string& fbName, int width, int height);
FramebufferRef getFramebuffer(const std::string& fbName) const ;
void unloadTexture(TextureRef ref, bool allowUnloadAtlas = false);

public:
AssetAPI* assetAPI;

bool isEntityVisible(Entity e, int cameraIndex = -1) const;
bool isVisible(const TransformationComponent* tc, int cameraIndex = -1) const;

void reloadTextures();
void reloadEffects();

void render();
void waitDrawingComplete();

public:
int windowW, windowH;
float screenW, screenH;

struct RenderCommand;
struct RenderQueue;

struct Atlas {
	std::string name;
	InternalTexture glref;
};

struct Framebuffer {
    GLuint fbo, rbo, texture;
    int width, height;
};


std::set<int> delayedAtlasIndexLoad;
std::set<InternalTexture> delayedDeletes;
std::vector<Atlas> atlas;
FramebufferRef nextValidFBRef;
std::map<std::string, FramebufferRef> nameToFramebuffer;
std::map<FramebufferRef, Framebuffer> ref2Framebuffers;

bool newFrameReady, frameQueueWritable;
int currentWriteQueue;
RenderQueue* renderQueue;
#ifdef USE_VBO
public:
GLuint squareBuffers[3];
private:
#endif

#ifndef EMSCRIPTEN
pthread_mutex_t mutexes[3];
pthread_cond_t cond[2];
#endif
#ifdef USE_VBO
public:
#endif
struct Shader {
	GLuint program;
	GLuint uniformMatrix, uniformColorSampler, uniformAlphaSampler, uniformColor, uniformCamera;
	#ifdef USE_VBO
	GLuint uniformUVScaleOffset, uniformRotation, uniformScaleZ;
	#endif
};
private:
Shader defaultShader, defaultShaderNoAlpha, defaultShaderEmpty;
GLuint whiteTexture;

TextureLibrary textureLibrary;
EffectRef nextEffectRef;
std::map<std::string, EffectRef> nameToEffectRefs;
std::map<EffectRef, Shader> ref2Effects;

bool initDone;

private:
GLuint compileShader(const std::string& assetName, GLuint type);
void loadTexture(const std::string& assetName, Vector2& realSize, Vector2& pow2Size, InternalTexture& out);
void drawRenderCommands(RenderQueue& commands);
void processDelayedTextureJobs();

public:
static void loadOrthographicMatrix(float left, float right, float bottom, float top, float near, float far, float* mat);
Shader buildShader(const std::string& vs, const std::string& fs);
EffectRef changeShaderProgram(EffectRef ref, bool firstCall, const Color& color, const TransformationComponent& cameraTransf, bool colorEnabled = true);
const Shader& effectRefToShader(EffectRef ref, bool firstCall, bool colorEnabled);
Vector2 getTextureSize(const std::string& textureName) const;
void removeExcessiveFrames(int& readQueue, int& writeQueue);

void setFrameQueueWritable(bool b);
typedef std::pair<GLuint, GLuint> ColorAlphaTextures;
ColorAlphaTextures chooseTextures(const InternalTexture& tex, const FramebufferRef& fbo, bool useFbo);
};
