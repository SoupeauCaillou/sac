#include "SimpleBehavior.h"
#include "util/Random.h"

#include "util/IntersectionUtil.h"
#include "systems/PhysicsSystem.h"

namespace Steering
{

template<>
void behavior(Entity e, const SeekParams& param, Context* interest, Context* priority, Context*) {
    if (param.target == 0)
        return;

    glm::vec2 diff = glm::normalize(TRANSFORM(param.target)->position - TRANSFORM(e)->position);

    float rotation = TRANSFORM(e)->rotation;
    for (int i=0; i<8; i++) {
        float d = glm::dot(diff, Steering::direction(rotation, i));

            // to allow 'e' to go backward if forward path is blocked
        if (d <0) {
            d = 0.3;
        } else {
            d = glm::min(1.0f, 0.3f + d);
        }
        if (d >= interest->directions[i]) {
            interest->directions[i] = d;
            priority->directions[i] = 1.0f;
            #if SAC_DEBUG
            interest->entities[i] = param.target;
            #endif
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

    float rotation = TRANSFORM(e)->rotation;
    for (int i=0; i<8; i++) {
        float d = glm::dot(diff, Steering::direction(rotation, i));

        if (d > 0) {
            danger->directions[i] = glm::min(1.0f, glm::max(d * coeff, danger->directions[i]));
            #if SAC_DEBUG
            danger->entities[i] = param.target;
            #endif
        }
    }
}

template<>
void behavior(Entity e, const AvoidParams& param, Context*, Context*, Context* danger) {
    if (param.count == 0)
        return;

    const auto* pc = PHYSICS(e);
    float speed = glm::length(pc->linearVelocity);
    const auto* tc = TRANSFORM(e);
#define FramePrediction 0.3f

    // assume obstacle is not-moving
    // we calculate predicted new pos of e for each direction
    // if we intersect with an object, fill danger map

    // compute size of 'e' upon 2 frame
    // example: [0]---[1] (pos at 0 and 1)
    // result:  [*******]
    const glm::vec2 interpSize = glm::vec2(tc->size.x * (1 + speed * FramePrediction), tc->size.y);

    for (int i=0; i<8; i++) {
        // calculate new pos
        float maxSpeed = 5;
        float maxForce = 30;
        // glm::vec2 movement = Steering::direction(tc->rotation, i) * speed * FramePrediction;
        glm::vec2 accel = Steering::direction(tc->rotation, i) * maxForce / pc->mass;
        glm::vec2 movement = (pc->linearVelocity + accel * FramePrediction) * FramePrediction;
        glm::vec2 nextFramePosition = tc->position + movement;
        Draw::Point(nextFramePosition);
        glm::vec2 interpPos = (tc->position + nextFramePosition) * 0.5f;
        Draw::Point(interpPos, Color(0, 0, 0));
        float rotation = Steering::angle(tc->rotation, i);


        for (int j=0; j<param.count; j++) {
            Entity avoid = param.entities[j];
            const auto* ta = TRANSFORM(avoid);

            if (IntersectionUtil::rectangleRectangle(interpPos, interpSize, rotation,
                ta->position, ta->size, ta->rotation)) {

                danger->directions[i] = glm::min(1.0f, glm::length(interpSize) / glm::max(ta->size.x, ta->size.y));
                #if SAC_DEBUG
                danger->entities[i] = avoid;
                #endif
            }
        }
    }
}

#if 0
static bool rectangleRectangle(const glm::vec2& rectAPos, const glm::vec2& rectASize, float rectARot,
            const glm::vec2& rectBPos, const glm::vec2& rectBSize, float rectBRot);


        glm::vec2 diff = TRANSFORM(avoid)->position - TRANSFORM(e)->position;
        float sizes =
            glm::max(TRANSFORM(avoid)->size.x, TRANSFORM(avoid)->size.y) +
            glm::max(TRANSFORM(e)->size.x, TRANSFORM(e)->size.y);
        float length = glm::length(diff);

        if (length > sizes * 1.5)
            continue;

        diff /= length;

        float rotation = TRANSFORM(e)->rotation;
        for (int j=0; j<8; j++) {
            float d = glm::dot(diff, Steering::direction(rotation, j));

            if (d > danger->directions[j]) {
                danger->directions[j] = d;
                #if SAC_DEBUG
                danger->entities[j] = avoid;
                #endif
            }
        }
    }
}
#endif
}
