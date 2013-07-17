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
#include <base/Color.h>

#include "../api/AssetAPI.h"

#include "System.h"
#include "opengl/Polygon.h"

typedef int FramebufferRef;
#define DefaultFrameBufferRef -1

typedef int VerticesRef;
// Default represents 'unit' shape (1 unit-length square for instance)
#define DefaultVerticesRef -1

struct TransformationComponent;

struct RenderingComponent {
	RenderingComponent() :
        texture(InvalidTextureRef),
        effectRef(DefaultEffectRef),
        color(Color()),
        shape(Shape::Square),
        dynamicVertices(DefaultVerticesRef),
        show(false), mirrorH(false), zPrePass(false), fastCulling(false),
        opaqueType(NON_OPAQUE),
        cameraBitMask(1)
        {
        fbo = false;
    }

    union {
	    TextureRef texture;
        FramebufferRef framebuffer;
    };
	EffectRef effectRef;
	Color color;
    Shape::Enum shape;
    VerticesRef dynamicVertices;
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

    typedef std::pair<GLuint, GLuint> ColorAlphaTextures;

#if SAC_DEBUG
    struct Stats {
        unsigned count;
        unsigned area;

        void reset() {
            count = area = 0;
        }
    } renderingStats[3];
#endif

public:
    ~RenderingSystem();

    void init();
    void setWindowSize(int w, int h, float sW, float sH);
    void setWindowSize(const glm::vec2& windowSize, const glm::vec2& screenSize);

    void loadAtlas(const std::string& atlasName, bool forceImmediateTextureLoading = false);
    void unloadAtlas(const std::string& atlasName);
    void invalidateAtlasTextures();

    int saveInternalState(uint8_t** out);
    void restoreInternalState(const uint8_t* in, int size);

    TextureRef loadTextureFile(const std::string& assetName);
    FramebufferRef createFramebuffer(const std::string& fbName, int width, int height);
    FramebufferRef getFramebuffer(const std::string& fbName) const ;
    void unloadTexture(TextureRef ref, bool allowUnloadAtlas = false);

    bool isVisible(Entity e) const;
    bool isVisible(const TransformationComponent* tc) const;

    void reloadTextures();

    void render();
    void waitDrawingComplete();

    EffectRef changeShaderProgram(EffectRef ref, bool firstCall, const Color& color, const glm::mat4& mvp,
        bool colorEnabled = true);
    const Shader& effectRefToShader(EffectRef ref, bool firstCall, bool colorEnabled);
    const glm::vec2& getTextureSize(const std::string& textureName);
    void removeExcessiveFrames(int& readQueue, int& writeQueue);

    ColorAlphaTextures chooseTextures(const InternalTexture& tex, const FramebufferRef& fbo, bool useFbo);

public:
    void enableRendering();
    void disableRendering();

    AssetAPI* assetAPI;

    int windowW, windowH;
    float screenW, screenH;

    std::vector<Atlas> atlas;
    FramebufferRef nextValidFBRef;
    std::map<std::string, FramebufferRef> nameToFramebuffer;
    std::map<FramebufferRef, Framebuffer> ref2Framebuffers;

    bool newFrameReady, frameQueueWritable;
    int currentWriteQueue;
    RenderQueue* renderQueue;

    TextureLibrary textureLibrary;
    EffectLibrary effectLibrary;

private:

void setFrameQueueWritable(bool b);

#if ! SAC_EMSCRIPTEN
    std::mutex *mutexes;
    std::condition_variable *cond;
#endif

    bool initDone;
private:
    void drawRenderCommands(RenderQueue& commands);
    void processDelayedTextureJobs();
    EffectRef defaultShader, defaultShaderNoAlpha, defaultShaderEmpty;
    GLuint whiteTexture;

#if SAC_LINUX && SAC_DESKTOP
    //reload on runtime .fs files when modified
    void updateReload();
#endif
public:
#if SAC_USE_VBO
    GLuint glBuffers[3];
#else
    GLuint glBuffers[1];
#endif
    std::vector<std::vector<glm::vec2> > dynamicVertices;
    Polygon shapes[Shape::Count];

    void defineDynamicVertices(unsigned idx, const std::vector<glm::vec2>& v);
};
