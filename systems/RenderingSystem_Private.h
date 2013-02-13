#pragma once

#if !defined(ANDROID) && !defined(EMSCRIPTEN)
#define CHECK_GL_ERROR
#endif

#ifdef CHECK_GL_ERROR
 #define GL_OPERATION(x) \
     (x); \
     RenderingSystem::check_GL_errors(#x);
#else
 #define GL_OPERATION(x) \
     (x);
#endif

#define EndFrameMarker -10
#define BeginFrameMarker -12

#define EnableZWriteBit (0x1 << 0)
#define DisableZWriteBit (0x1 << 1)
#define EnableBlendingBit (0x1 << 2)
#define DisableBlendingBit (0x1 << 3)
#define EnableColorWriteBit (0x1 << 4)
#define DisableColorWriteBit (0x1 << 5)

enum {
    ATTRIB_VERTEX = 0,
    ATTRIB_UV,
    ATTRIB_POS_ROT,
    ATTRIB_SCALE,
    NUM_ATTRIBS
};

#define L_RENDER  0
#define L_QUEUE   1
#define L_TEXTURE 2

#define C_RENDER_DONE 0
#define C_FRAME_READY 1

struct RenderingSystem::RenderQueue {
    RenderQueue() : count(0) {}
    uint16_t count;
    std::vector<RenderCommand> commands;
};

struct RenderingSystem::RenderCommand {
    float z;
    EffectRef effectRef;
    union {
        TextureRef texture;
        FramebufferRef framebuffer;
        InternalTexture glref;
    };
    unsigned int rotateUV;
    Vector2 uv[2];
    Vector2 halfSize;
    Color color;
    Vector2 position;
    float rotation;
    int flags;
    bool mirrorH, fbo;
};

class CameraComponent;

void packCameraAttributes(
    const TransformationComponent* cameraTrans,
    const CameraComponent* cameraComp,
    RenderingSystem::RenderCommand& out);

void unpackCameraAttributes(
    const RenderingSystem::RenderCommand& in,
    TransformationComponent* cameraTrans,
    CameraComponent* cameraComp);

