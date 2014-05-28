/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#pragma once

#include <condition_variable>
#include <mutex>

#include "opengl/OpenglHelper.h"
#include "opengl/TextureLibrary.h"
#include "opengl/EffectLibrary.h"

#include <glm/glm.hpp>
#include <base/Color.h>

#include "System.h"
#include "opengl/GLState.h"

typedef uint8_t FramebufferRef;
#define DefaultFrameBufferRef 0

typedef int VerticesRef;
// Default represents 'unit' shape (1 unit-length square for instance)
#define DefaultVerticesRef -1

struct TransformationComponent;
struct GLState;
struct VertexData;

namespace RenderingFlags
{
    const uint8_t NonOpaque        = 0x01;
    const uint8_t MirrorHorizontal = 0x02;
    const uint8_t ZPrePass         = 0x04;
    const uint8_t Constant         = 0x08;
    const uint8_t FastCulling      = 0x10;
    const uint8_t TextureIsFBO     = 0x20;
    const uint8_t ConstantNeedsUpdate = 0x40;
}

struct RenderingComponent {
    RenderingComponent() :
        texture(InvalidTextureRef),
        show(false),
        flags(0),
        indiceOffset(0),
        effectRef(DefaultEffectRef),
        cameraBitMask(1),
        color(Color())
        {}

    union {
        TextureRef texture; // 32 bits
        FramebufferRef framebuffer;
    };
    uint8_t show;           // 8 bits
    uint8_t flags;          // 8 bits
    uint16_t indiceOffset; // 16
    EffectRef effectRef;    // 8 bits
    uint8_t cameraBitMask;  // 8 bits
    Color color;            // 128 bits
};

#define theRenderingSystem RenderingSystem::GetInstance()
#if SAC_DEBUG
#define RENDERING(e) theRenderingSystem.Get(e,true,__FILE__,__LINE__)
#else
#define RENDERING(e) theRenderingSystem.Get(e)
#endif

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

    TextureRef loadTextureFile(const char* assetName);
    FramebufferRef createFramebuffer(const std::string& fbName, int width, int height);
    FramebufferRef getFramebuffer(const std::string& fbName) const ;
    void unloadTexture(TextureRef ref, bool allowUnloadAtlas = false);

    bool isVisible(Entity e) const;
    bool isVisible(const TransformationComponent* tc) const;

    void reloadTextures();

    void render();
    void waitDrawingComplete();

    EffectRef changeShaderProgram(EffectRef ref, const Color& color, const glm::mat4& mvp);
    glm::vec2 getTextureSize(const char* textureName);
    glm::vec2 getTextureSize(const TextureRef& textureRef);
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
EffectRef chooseDefaultShader(bool alphaBlendingOn, bool colorEnabled, bool hasTexture) const;

#if ! SAC_EMSCRIPTEN
    std::mutex *mutexes;
    std::condition_variable *cond;
#endif

    bool initDone;
private:
    void drawRenderCommands(RenderQueue& commands);
    void processDelayedTextureJobs();
    EffectRef defaultShader, defaultShaderNoAlpha, defaultShaderEmpty, defaultShaderNoTexture;
    GLuint whiteTexture;

#if SAC_LINUX && SAC_DESKTOP
    //reload on runtime .fs files when modified
    void updateReload();
#endif
public:
    GLuint glBuffers[3];

#if SAC_INGAME_EDITORS
    struct {
        bool zPrePass;
        bool nonOpaque;
        bool opaque;
        bool runtimeOpaque;
    } highLight;

    bool wireframe;
#endif
    uint16_t nextConstantOffset;

private:
    // GL state
    GLState glState;
#if SAC_ANDROID || SAC_EMSCRIPTEN
    bool hasDiscardExtension;
    PFNGLDISCARDFRAMEBUFFEREXTPROC glDiscardFramebufferEXT;
#endif
    VertexData* vertices;
    unsigned short* indices;
};
