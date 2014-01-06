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

#include "System.h"
#include <functional>
#if SAC_DEBUG
#include "base/Frequency.h"
#endif

struct CollisionComponent {
    CollisionComponent() :
        group(0), collideWith(0),
        restorePositionOnCollision(false), isARay(false), rayTestDone(false),
        previousPosition(0.0f), previousRotation(0.0f),
        collidedWithLastFrame(0), collisionAt(0.0f) {}
    int group;
    int collideWith;
    bool restorePositionOnCollision, isARay, rayTestDone;

    glm::vec2 previousPosition;
    float previousRotation;

    Entity collidedWithLastFrame;
    glm::vec2 collisionAt;
};

#define theCollisionSystem CollisionSystem::GetInstance()
#if SAC_DEBUG
#define COLLISION(e) theCollisionSystem.Get(e,true,__FILE__,__LINE__)
#else
#define COLLISION(e) theCollisionSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Collision)

    public:
        glm::vec2 worldSize;
#if SAC_DEBUG
        bool showDebug;
        float maximumRayCastPerSec, maximumRayCastPerSecAccum;
#endif
    private:
        std::vector<Entity> debug;
};
