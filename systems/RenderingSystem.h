#pragma once

#include <cstdio>
#include <cstdlib>
#include <algorithm>

#include <thread>
#include <condition_variable>
#include <mutex>


#include "opengl/OpenglHelper.h"
#include "opengl/TextureLibrary.h"
#include "opengl/EffectLibrary.h"
#include <set>
#include <queue>
#include <list>

#include <glm/glm.hpp>
#include "base/Color.h"
#include "../api/AssetAPI.h"

#include "System.h"

typedef int FramebufferRef;
#define DefaultFrameBufferRef -1

struct TransformationComponent;

struct RenderingComponent {
	RenderingComponent() : texture(InvalidTextureRef), effectRef(DefaultEffectRef), color(Color()), show(false), mirrorH(false), zPrePass(false), fastCulling(false), opaqueType(NON_OPAQUE), cameraBitMask(~0u) {
        fbo = false;
    }

    union {
	    TextureRef texture;
        FramebufferRef framebuffer;
    };
	EffectRef effectRef;
	Color color;
	bool show, mirrorH, zPrePass, fastCulling, fbo;
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
FramebufferRef createFramebuffer(const std::string& fbName, int width, int height);
FramebufferRef getFramebuffer(const std::string& fbName) const ;
void unloadTexture(TextureRef ref, bool allowUnloadAtlas = false);

public:
AssetAPI* assetAPI;

bool isEntityVisible(Entity e, int cameraIndex = -1) const;
bool isVisible(const TransformationComponent* tc, int cameraIndex = -1) const;

void reloadTextures();

void render();
void waitDrawingComplete();

public:
int windowW, windowH;
float screenW, screenH;

struct RenderCommand;
struct RenderQueue;

struct Atlas {
	std::string name;
    TextureRef ref;
	// InternalTexture glref;
};

struct Framebuffer {
    GLuint fbo, rbo, texture;
    int width, height;
};


std::vector<Atlas> atlas;
FramebufferRef nextValidFBRef;
std::map<std::string, FramebufferRef> nameToFramebuffer;
std::map<FramebufferRef, Framebuffer> ref2Framebuffers;

bool newFrameReady, frameQueueWritable;
int currentWriteQueue;
RenderQueue* renderQueue;
#ifdef SAC_USE_VBO
public:
GLuint squareBuffers[3];
private:
#endif

#ifndef SAC_EMSCRIPTEN
std::mutex *mutexes;
std::condition_variable *cond;
#endif
#ifdef SAC_USE_VBO
public:
#endif

TextureLibrary textureLibrary;
EffectLibrary effectLibrary;

private:
EffectRef defaultShader, defaultShaderNoAlpha, defaultShaderEmpty;
GLuint whiteTexture;

bool initDone;

private:

void drawRenderCommands(RenderQueue& commands);
void processDelayedTextureJobs();

#if defined(SAC_LINUX) & defined(SAC_DESKTOP)
void updateInotify();
#endif

public:
static void loadOrthographicMatrix(float left, float right, float bottom, float top, float near, float far, float* mat);

EffectRef changeShaderProgram(EffectRef ref, bool firstCall, const Color& color, const TransformationComponent& cameraTransf, bool colorEnabled = true);
const Shader& effectRefToShader(EffectRef ref, bool firstCall, bool colorEnabled);
const glm::vec2& getTextureSize(const std::string& textureName);
void removeExcessiveFrames(int& readQueue, int& writeQueue);

void setFrameQueueWritable(bool b);
typedef std::pair<GLuint, GLuint> ColorAlphaTextures;
ColorAlphaTextures chooseTextures(const InternalTexture& tex, const FramebufferRef& fbo, bool useFbo);
};
