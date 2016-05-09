#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

float aim(const glm::vec2& position, float rotation, const glm::vec2& target) {
    glm::vec2 diff = target - position;
    float targetAngle = atan2(diff.y, diff.x);

    float diff1 = targetAngle - rotation;
    float diff2 = -glm::sign(diff1) * glm::pi<float>() * 2.0f + diff1;

    return glm::abs(diff1) < glm::abs(diff2) ?
        diff1 : diff2;
}
