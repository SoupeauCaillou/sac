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

#include <glm/glm.hpp>

#include "System.h"

namespace Cardinal {
    enum Enum {
        NW, N, NE,
        W,  C, E,
        SW, S, SE
    };
}

struct AnchorComponent {
    AnchorComponent(): parent(0), position(0.0f), anchor(0.0f), rotation(0.0f), z(0) {}

    // parent
    Entity parent;
    // position in parent's coordinates
    glm::vec2 position;
    // anchor point in our own coords
    glm::vec2 anchor;
    // rotation around anchor
    float rotation;
    // z offset from parent
    float z;
};

struct TransformationComponent;

#define theAnchorSystem AnchorSystem::GetInstance()
#if SAC_DEBUG
#define ANCHOR(e) theAnchorSystem.Get(e,true,__FILE__,__LINE__)
#else
#define ANCHOR(e) theAnchorSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Anchor)

public:
    static glm::vec2 adjustPositionWithAnchor(const glm::vec2& position, const glm::vec2& anchor);
    static glm::vec2 adjustPositionWithCardinal(const glm::vec2& position, const glm::vec2& size, Cardinal::Enum cardinal);
    static void adjustTransformWithAnchor(TransformationComponent* tc, const TransformationComponent* parentTc, const AnchorComponent* ac);

#if SAC_DEBUG
    void Delete(Entity e) override;
#endif
};
