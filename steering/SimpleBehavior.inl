#include "SimpleBehavior.h"
#include "util/Random.h"

namespace Steering
{

template<>
void behavior(Entity e, const SeekParams& param, Context* interest, Context* priority, Context*) {
    if (param.target == 0)
        return;

    glm::vec2 diff = glm::normalize(TRANSFORM(param.target)->position - TRANSFORM(e)->position);

    for (int i=0; i<8; i++) {
        float d = glm::dot(diff, Steering::direction(i));

        if (d >= interest->directions[i]) {
            interest->directions[i] = d;
            priority->directions[i] = 1.0f;
        } else if (0) {
            interest->directions[i] = Random::Float(0.05, 0.1);
            priority->directions[i] = 1.0f;
        }
    }
}


template<>
void behavior(Entity e, const FleeParams& param, Context*, Context*, Context* danger) {
    if (param.target == 0)
        return;

    glm::vec2 diff = TRANSFORM(param.target)->position - TRANSFORM(e)->position;
    float sizes =
        glm::max(TRANSFORM(param.target)->size.x, TRANSFORM(param.target)->size.y) +
        glm::max(TRANSFORM(e)->size.x, TRANSFORM(e)->size.y);
    float length = glm::length(diff);
    diff /= length;
    float distance = length - sizes;

    if (distance > param.radius)
        return;

    float coeff = 1.0f - distance / param.radius;

    for (int i=0; i<8; i++) {
        float d = glm::dot(diff, Steering::direction(i));

        if (d > 0) {
            danger->directions[i] = glm::min(1.0f, glm::max(d * coeff, danger->directions[i]));
        }
    }
}

}
