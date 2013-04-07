#pragma once

#define EndFrameMarker -10
#define BeginFrameMarker -12

#define EnableZWriteBit (0x1 << 0)
#define DisableZWriteBit (0x1 << 1)
#define EnableBlendingBit (0x1 << 2)
#define DisableBlendingBit (0x1 << 3)
#define EnableColorWriteBit (0x1 << 4)
#define DisableColorWriteBit (0x1 << 5)

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
    glm::vec2 uv[2];
    glm::vec2 halfSize;
    Color color;
    glm::vec2 position;
    float rotation;
    int flags;
    bool mirrorH, fbo;
};

struct CameraComponent;

void packCameraAttributes(
    const TransformationComponent* cameraTrans,
    const CameraComponent* cameraComp,
    RenderingSystem::RenderCommand& out);

void unpackCameraAttributes(
    const RenderingSystem::RenderCommand& in,
    TransformationComponent* cameraTrans,
    CameraComponent* cameraComp);
