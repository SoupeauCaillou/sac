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

#if !DISABLE_COLLISION_SYSTEM
#include "System.h"
#include <functional>
#if SAC_DEBUG
#include "base/Frequency.h"
#endif

struct TransformationComponent;

struct CollisionComponent {
    CollisionComponent() :
        group(0), collideWith(0),
        // previousPosition(0.0f), previousRotation(0.0f),
        ignore(0) { collision.count = 0;
            restoreTransformation.atCollision =
            restoreTransformation.beforeCollision = false;
        #if 0
        ray.is = false; ray.testDone = false; ray.maxCollision = 1;
        #endif
    }
    int group;
    // bitfield to indicate which groups to test for collision
    // special value 0 indicates static entity
    uint32_t collideWith;

    struct {
        bool beforeCollision;
        bool atCollision;
    } restoreTransformation;

#if 0
    glm::vec2 previousPosition;
    float previousRotation;
    struct {
        bool is;
        bool testDone;
        int maxCollision;
    } ray;
#endif
    struct {
        int count;
        Entity* with;
        float* at;
        glm::vec2* normal; /* only valid for objects */
    } collision;
    Entity ignore; /* TODO ignore several entities */
};

#define theCollisionSystem CollisionSystem::GetInstance()
#if SAC_DEBUG
#define COLLISION(e) theCollisionSystem.Get(e, true, __FILE__, __LINE__)
#else
#define COLLISION(e) theCollisionSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Collision)

public:
static glm::vec2 collisionPointToNormal(const glm::vec2& point,
                                        const TransformationComponent* tc);

#if SAC_DEBUG
bool showDebug;
int maximumRayCastPerSec;
float maximumRayCastPerSecAccum;

private:
std::vector<Entity> debug;
#endif
std::vector<Entity> collisionEntity;
std::vector<float> collisionTimestamp;
std::vector<glm::vec2> collisionNormal;
}
;
#endif
