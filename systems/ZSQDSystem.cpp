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

#if !DISABLE_ZSQD_SYSTEM

#include "ZSQDSystem.h"

#include "AnchorSystem.h"
#include "CollisionSystem.h"

#include "base/Log.h"
#include "util/Draw.h"
#include "util/IntersectionUtil.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/compatibility.hpp>
#include "util/SerializerProperty.h"

INSTANCE_IMPL(ZSQDSystem);

ZSQDSystem::ZSQDSystem() : ComponentSystemImpl<ZSQDComponent>(HASH("ZSQD", 0xbecf877c), ComponentType::Complex) {
    ZSQDComponent zc;
    componentSerializer.add(new Property<float>(HASH("friction_coeff", 0x3efd422a), OFFSET(frictionCoeff, zc), 0.0001f));
    componentSerializer.add(new Property<float>(HASH("max_speed", 0x3fbe6552), OFFSET(maxSpeed, zc), 0.0001f));
    componentSerializer.add(new Property<float>(HASH("new_direction_coeff", 0x62c7b51a), OFFSET(newDirectionCoeff, zc), 0.0001f));
    componentSerializer.add(new Property<float>(HASH("rotation_speed", 0x201b183c), OFFSET(rotationSpeed, zc), 0.0001f));
    componentSerializer.add(new Property<float>(HASH("rotation_speed_stopped", 0xea13d518), OFFSET(rotationSpeedStopped, zc), 0.0001f));
    componentSerializer.add(new Property<bool>(HASH("rotate_to_face_direction", 0xaac25ee9), OFFSET(rotateToFaceDirection, zc)));
    componentSerializer.add(new Property<int>(HASH("collide_with", 0x6b658240), OFFSET(collideWith, zc), 0));

#if SAC_DEBUG
    showDebug = false;
#endif
}

namespace Pass {
    enum Enum {
        First = 0,
        Second,
        Count
    };
}
static Pass::Enum pass = Pass::First;

void ZSQDSystem::DoUpdate(float dt) {
    #if SAC_DEBUG
    if (showDebug) Draw::Clear(HASH("ZSQD", 0xbecf877c));
    #endif

    #if 0
    if (pass == Pass::Second) {
        FOR_EACH_ENTITY_COMPONENT(ZSQD, a, zc)
        /* check collision */
        if (zc->collideWith > 0) {
            const auto* cc = COLLISION(a);
            #if SAC_DEBUG
            if (showDebug) Draw::Point(HASH("ZSQD", 0xbecf877c), cc->previousPosition, Color(0, 0, 1));
            #endif

            const glm::vec2 displacement = TRANSFORM(a)->position - zc->previousPosition;
            Draw::Vec2(TRANSFORM(a)->position, displacement, Color(1, 0, 1));

            glm::vec2 bestNewPosition = TRANSFORM(a)->position;
            float minCancellation = -FLT_MAX;

            LOGI_IF(cc->collision.count, __(cc->collision.count) << ' ' << __(displacement) << ' ' << TRANSFORM(a)->position);
            for (int i=0; i<cc->collision.count; i++) {
                const glm::vec2& at = cc->collision.at[i];
                const glm::vec2& normal = cc->collision.normal[i];

                if (glm::dot(normal, displacement) >= 0) {
                    continue;
                }

                if (!IntersectionUtil::rectangleRectangle(
                        TRANSFORM(cc->collision.with[i]),
                        bestNewPosition,
                        TRANSFORM(a)->size,
                        TRANSFORM(a)->rotation)) {
                    continue;
                }

                const glm::vec2 b = at - TRANSFORM(a)->position;
                float overshot = glm::sqrt(glm::pow(TRANSFORM(a)->size.x, 2.0f) + glm::pow(TRANSFORM(a)->size.y, 2.0f)) * 0.5 + glm::dot(b, normal);
                float d = -overshot;

                LOGI(__(i) << ':' << __(cc->collision.with[i]) << ',' << __(normal) << ',' << d << ' ' << (d > minCancellation));
                Draw::Vec2(cc->collision.at[i], normal, Color(0.5, 0.5, 0.5));

                glm::vec2 proposition = bestNewPosition - (d-0.01f) * normal;
                bool avoidCollision =
                    !IntersectionUtil::rectangleRectangle(
                        TRANSFORM(cc->collision.with[i]),
                        proposition,
                        TRANSFORM(a)->size,
                        TRANSFORM(a)->rotation);
                // only consider if normal is opposite
                if (avoidCollision) {
                    bestNewPosition = proposition;
                }
            }

            bool valid = true;
            int i=0;
            for (i=0; i<cc->collision.count && valid; i++) {
                valid &= !IntersectionUtil::rectangleRectangle(
                    TRANSFORM(cc->collision.with[i]),
                        bestNewPosition,
                        TRANSFORM(a)->size,
                        TRANSFORM(a)->rotation);
            }

            if (valid) {
                TRANSFORM(a)->position = bestNewPosition;
            } else {
                LOGI("failed at: " << __(i) << '/' << minCancellation << ' ' << __(bestNewPosition));
                TRANSFORM(a)->position = zc->previousPosition;
            }



            #if SAC_DEBUG
            if (showDebug) Draw::Vec2(HASH("ZSQD", 0xbecf877c), TRANSFORM(a)->position, zc->currentDirection, Color(0, 1, 1));
            #endif
        }
        END_FOR_EACH()
    }
    #endif
    if (pass == Pass::First) {
        FOR_EACH_ENTITY_COMPONENT(ZSQD, a, zc)
        zc->previousPosition = TRANSFORM(a)->position;

        //decrease current speed
        zc->currentSpeed = glm::max(zc->currentSpeed - zc->frictionCoeff * zc->maxSpeed * dt, 0.f);

        //if someone added some new directions, update speed and direction
        if (!zc->directions.empty()) {
            //LOGI(zc->directions.size() << " more directions");

            //calculate the average new direction from the whole new inputs
            glm::vec2 newDir(0, 0);
            for (auto it : zc->directions) {
                float norm = it.x * it.x + it.y * it.y;

                newDir.x += it.x / norm;
                newDir.y += it.y / norm;
            }
            //normalize the new direction
            if (glm::length2(newDir) < 0.001) {
                // no change
                continue;
            }

            newDir = glm::normalize(newDir);

            //LOGI("current dir: " << zc->currentDirection << " at speed " << zc->currentSpeed
            //    << "\nAnd new direction is: " << newDir << " at speed " << zc->maxSpeed);

            //calculate the weight of the new direction
            float weight = zc->newDirectionCoeff;
            if (newDir == glm::vec2(0.f, 0.f))
                weight = zc->newDirectionCoeff * 3;
            else if (zc->currentDirection == glm::vec2(0.f, 0.f))
                weight = 1.;

            //update the direction with ponderated directions
            zc->currentDirection += newDir * weight * dt;// + zc->currentDirection * (1 - weight);

            //currentDirection norm must be <= 1
            if (glm::length(zc->currentDirection) > 1.f)
                zc->currentDirection = glm::normalize(zc->currentDirection);

            //reset speed to maxSpeed, because we had new inputs
            zc->currentSpeed = zc->maxSpeed;
        }

        //if we are moving, update the position
        if (zc->currentSpeed > 0.f) {
            if (zc->currentDirection != glm::vec2(0.f, 0.f)) {
                TRANSFORM(a)->position += zc->currentDirection * zc->currentSpeed * dt;
            }
        } else {
            zc->currentDirection = glm::vec2(0.f, 0.f);
        }

        if (zc->rotateToFaceDirection && zc->currentSpeed > 0.01) {
            const float t = glm::atan2<float, glm::mediump>(zc->currentDirection.y, zc->currentDirection.x);
            float diffRot = t - TRANSFORM(a)->rotation;
            if (glm::abs(diffRot) > glm::pi<float>()) {
                diffRot = diffRot - glm::sign(diffRot) * glm::pi<float>() * 2;
            }
            TRANSFORM(a)->rotation += diffRot * zc->rotationSpeed * dt;
        }
        zc->directions.clear();

        #if SAC_DEBUG
        if (showDebug) Draw::Vec2(HASH("ZSQD", 0xbecf877c), TRANSFORM(a)->position, zc->currentDirection, Color(0, 1, 0));
        #endif
        END_FOR_EACH()
    }

    pass = (Pass::Enum) (((int)pass + 1) % Pass::Count);
}
#endif
