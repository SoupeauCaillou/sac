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

// Possible flags
#define ZPrePassFlagSet       0x1
#define OpaqueFlagSet         0x5
#define AlphaBlendedFlagSet   0x6
#define DebugFlagSet          0x7

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
    uint64_t key;
    union {
        float z;
        int zi;
    };
    EffectRef effectRef;
    union {
        TextureRef texture;
        FramebufferRef framebuffer;
        InternalTexture glref;
    };
    int atlasIndex;
    glm::vec2 uv[2];
    glm::vec2 halfSize;
    Color color;
    glm::vec2 position;
    float rotation;
    int flags, shapeType;
    uint8_t rflags;
    bool rotateUV;
#if SAC_DEBUG
    Entity e;
#endif
};

struct CameraComponent;

struct VertexData {
    glm::vec3 position;
    glm::vec2 uv;
};

#define MAX_VERTEX_COUNT 512
#define MAX_INDICE_COUNT 1024

void packCameraAttributes(
    const TransformationComponent* cameraTrans,
    const CameraComponent* cameraComp,
    RenderingSystem::RenderCommand& out);

void unpackCameraAttributes(
    const RenderingSystem::RenderCommand& in,
    TransformationComponent* cameraTrans,
    CameraComponent* cameraComp);
