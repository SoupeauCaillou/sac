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

#include "base/Log.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/compatibility.hpp>

INSTANCE_IMPL(ZSQDSystem);

ZSQDSystem::ZSQDSystem() : ComponentSystemImpl<ZSQDComponent>(HASH("ZSQD", 0x0)) {
    ZSQDComponent zc;
    componentSerializer.add(new Property<float>(HASH("friction_coeff", 0x3efd422a), OFFSET(frictionCoeff, zc), 0.0001f));
    componentSerializer.add(new Property<float>(HASH("max_speed", 0x3fbe6552), OFFSET(maxSpeed, zc), 0.0001f));
    componentSerializer.add(new Property<float>(HASH("new_direction_coeff", 0x62c7b51a), OFFSET(newDirectionCoeff, zc), 0.0001f));
    componentSerializer.add(new Property<float>(HASH("rotation_speed", 0x201b183c), OFFSET(rotationSpeed, zc), 0.0001f));
    componentSerializer.add(new Property<float>(HASH("rotation_speed_stopped", 0xea13d518), OFFSET(rotationSpeedStopped, zc), 0.0001f));
    componentSerializer.add(new Property<bool>(HASH("rotate_to_face_direction", 0xaac25ee9), OFFSET(rotateToFaceDirection, zc)));
}

void ZSQDSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(ZSQD, a, zc)
        //decrease current speed
        zc->currentSpeed = glm::max(zc->currentSpeed - zc->frictionCoeff * zc->maxSpeed * dt, 0.f);

        if (false) {
            if (!zc->directions.empty()) {
                LOGI(zc->directions.size() << " more directions");

                //calculate the average new direction from the whole new inputs
                glm::vec2 newDir(0, 0);
                for (auto it : zc->directions) {
                    newDir += it;
                }

                if (!zc->lateralMove) {
                    if (glm::abs(newDir.y) > 0.001) {
                        zc->currentSpeed = zc->maxSpeed;
                        TRANSFORM(a)->position += glm::rotate(glm::vec2(0, newDir.y * zc->currentSpeed * dt), TRANSFORM(a)->rotation);
                    }
                    if (glm::abs(newDir.x) > 0.001) {
                        float rotSpeed = glm::lerp(zc->rotationSpeedStopped, zc->rotationSpeed, zc->currentSpeed / zc->maxSpeed);
                        TRANSFORM(a)->rotation -= glm::sign(newDir.x) * rotSpeed * dt;
                    }
                } else {
                    if (glm::length2(newDir) > 0.0001) {
                        zc->currentSpeed = zc->maxSpeed;
                        TRANSFORM(a)->position += glm::rotate(glm::normalize(newDir) * zc->currentSpeed * dt, TRANSFORM(a)->rotation);
                    }
                }
            }
        } else {
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
                if (zc->currentDirection != glm::vec2(0.f, 0.f))
                    TRANSFORM(a)->position += zc->currentDirection * zc->currentSpeed * dt;
            } else {
                zc->currentDirection = glm::vec2(0.f, 0.f);
            }
        }

        if (zc->rotateToFaceDirection && zc->currentSpeed > 0.01) {
            const float t = glm::atan2<float, glm::mediump>(zc->currentDirection.y, zc->currentDirection.x);
            float diffRot = t - TRANSFORM(a)->rotation;
            if (glm::abs(diffRot) > glm::pi<float>()) {
                diffRot = diffRot - glm::sign(diffRot) * glm::pi<float>() * 2;
            }
            TRANSFORM(a)->rotation += diffRot * 20 * dt;
        }
        zc->directions.clear();
    END_FOR_EACH()
}
#endif
