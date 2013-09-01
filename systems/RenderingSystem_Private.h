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
    RenderCommand();

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
    int flags, shapeType, vertices;
    bool mirrorH, fbo, padding[2];
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

#if SAC_USE_VBO
#define MAX_BATCH_TRIANGLE_COUNT 128
#else
#define MAX_BATCH_TRIANGLE_COUNT 128
#endif
