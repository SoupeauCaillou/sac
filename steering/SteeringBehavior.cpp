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

#include "SteeringBehavior.h"

#include <glm/gtc/constants.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>

static const float ContextAngle[8] = {
    0.0f,
    glm::pi<float>() * 0.20f,
    glm::pi<float>() * 0.40f,
    glm::pi<float>() * 0.70f,
    glm::pi<float>(),
    -glm::pi<float>() * 0.70f,
    -glm::pi<float>() * 0.40f,
    -glm::pi<float>() * 0.20f
};

static const glm::vec2 ContextDirection[8] = {
    glm::vec2(1.0000, 0.0000),
    glm::vec2(0.8090, 0.5878),
    glm::vec2(0.3090, 0.9511),
    glm::vec2(-0.5878, 0.8090),
    glm::vec2(-1.0000, -0.0000),
    glm::vec2(-0.5878, -0.8090),
    glm::vec2(0.3090, -0.9511),
    glm::vec2(0.8090, -0.5878),
};

namespace Steering
{
    const glm::vec2 direction(float rotation, int index) {
        LOGF_IF(index < 0 || index >= 8, "Invalid index value: " << index);
        return glm::rotate(ContextDirection[index], rotation);
    }

    float angle(float rotation, int index) {
        LOGF_IF(index < 0 || index >= 8, "Invalid index value: " << index);
        return rotation + ContextAngle[index];
    }
}
