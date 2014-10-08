#include "SteeringBehavior.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>

static const float ContextAngle[8] = {
    0.0f,
    glm::pi<float>() * 0.15f,
    glm::pi<float>() * 0.3f,
    glm::pi<float>() * 0.55f,
    glm::pi<float>(),
    -glm::pi<float>() * 0.55f,
    -glm::pi<float>() * 0.3f,
    -glm::pi<float>() * 0.15f

};

static const glm::vec2 ContextDirection[8] = {
    glm::vec2(1.0000, 0.0000),
    glm::vec2(0.8910, 0.4540),
    glm::vec2(0.5878, 0.8090),
    glm::vec2(-0.1564, 0.9877),
    glm::vec2(-1.0000, -0.0000),
    glm::vec2(-0.1564, -0.9877),
    glm::vec2(0.5878, -0.8090),
    glm::vec2(0.8910, -0.4540),
};

namespace Steering
{
    const glm::vec2 direction(float rotation, int index) {
        LOGF_IF(index < 0 || index >= 8, "Invalid index value: " << index);
        return glm::rotate(ContextDirection[index], rotation);
    }

    const float angle(float rotation, int index) {
        LOGF_IF(index < 0 || index >= 8, "Invalid index value: " << index);
        return rotation + ContextAngle[index];
    }
}
