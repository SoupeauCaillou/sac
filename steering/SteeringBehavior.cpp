#include "SteeringBehavior.h"
#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>

static const float ContextAngle[8] = {
    0.0f,
    glm::pi<float>() * 0.25f,
    glm::pi<float>() * 0.5f,
    glm::pi<float>() * 0.75f,
    glm::pi<float>(),
    glm::pi<float>() * 1.25f,
    glm::pi<float>() * 1.5f,
    glm::pi<float>() * 1.75f

};

static const glm::vec2 ContextDirection[8] = {
    glm::vec2(1.0f, 0.0f),
    glm::vec2(0.7071, 0.7071),
    glm::vec2(0.0f, 1.0f),
    glm::vec2(-0.7071, 0.7071),
    glm::vec2(-1.0f, 0.0f),
    glm::vec2(-0.7071, -0.7071),
    glm::vec2(0.0f, -1.0f),
    glm::vec2(0.7071, -0.7071)
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
