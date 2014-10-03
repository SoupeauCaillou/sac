#include "SteeringBehavior.h"


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
    const glm::vec2& direction(int index) {
        LOGF_IF(index < 0 || index >= 8, "Invalid index value: " << index);
        return ContextDirection[index];
    }
}
