#include "SimpleBehavior.h"
#include "util/Random.h"

#include "util/IntersectionUtil.h"
#include "systems/PhysicsSystem.h"
#if SAC_DEBUG
#include "systems/RenderingSystem.h"
#endif

namespace Steering
{

template<>
void behavior(Entity e, float, SeekParams& param, Context* interest, Context* priority, Context*) {
    if (param.count == 0)
        return;

    float closestTargetDistanceSquared = FLT_MAX;
    for (int j=0; j<param.count; j++) {
        Entity target = param.entities[j];

        glm::vec2 diff = TRANSFORM(target)->position - TRANSFORM(e)->position;
        float l2 = glm::length2(diff);

        if (l2 < closestTargetDistanceSquared)
            closestTargetDistanceSquared = l2;
    }

    for (int j=0; j<param.count; j++) {
        Entity target = param.entities[j];

        glm::vec2 diff = TRANSFORM(target)->position - TRANSFORM(e)->position;
        float l = glm::length(diff);
        diff /= l;
        float l2 = l * l;

        float rotation = TRANSFORM(e)->rotation;
        float w = param.weight ? param.weight[j] : 1.0f;

        // consider 8 move direction
        for (int i=0; i<8; i++) {
            float coeff = closestTargetDistanceSquared / l2;
            float d = w * coeff *
                glm::dot(diff, Steering::direction(rotation, i));

            if (d <0) {
                d = 0.05;
            } else {
                d = glm::min(1.0f, d);
            }
            if (d >= interest->directions[i]) {
                interest->directions[i] = d;
                priority->directions[i] = w;//0.25;//.0f;
                #if SAC_DEBUG
                interest->entities[i] = target;
                #endif
            }
        }
        // or consider stopping here
        {
            float overlapDistance = (
                glm::max(TRANSFORM(e)->size.x, TRANSFORM(e)->size.y) +
                glm::max(TRANSFORM(target)->size.x, TRANSFORM(target)->size.y)
                ) * 0.3;
            float d = 1.0 - (l - overlapDistance) / overlapDistance;
            //LOGI_IF(d > 0, d);
            if (d >= 0 && d >= interest->directions[8]) {
                    interest->directions[8] = d * w;
                    priority->directions[8] = 1.0f;
                    #if SAC_DEBUG
                    interest->entities[8] = target;
                    #endif
            }
        }
    }
}


template<>
void behavior(Entity e, float /*dt*/, FleeParams& param, Context* interest, Context* priority, Context* /*danger*/) {
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

#if 0
    static Entity eTarget = 0;
    if (!eTarget) {
        eTarget = theEntityManager.CreateEntity(HASH("__/flee", 0x7f43116b));
        ADD_COMPONENT(eTarget, Transformation);
        TRANSFORM(eTarget)->size = glm::vec2(0.01f);
    }
    SeekParams seek;
    seek.entities = &eTarget;
    seek.count = 1;
    TRANSFORM(eTarget)->position = TRANSFORM(e)->position - diff * (param.radius * Random::Float(1.0f, 2.0f));

    behavior(e, dt, seek, interest, priority, danger);
    return;
#endif

    float coeff = 1.0f - distance / param.radius;
    float rotation = TRANSFORM(e)->rotation;

    // add danger to directions pointing at target,
    // add interest otherwise
    float maxDotProduct = 0.0;
    for (int i=0; i<8; i++) {
        float d = glm::dot(diff, Steering::direction(rotation, i)) * coeff;
        maxDotProduct = glm::max(d, maxDotProduct);
    }
    maxDotProduct = 0;

    // move at all cost
    for (int i=0; i<8; i++) {
        float d = -glm::dot(diff, Steering::direction(rotation, i)) * coeff;
        d = (1 + d) * 0.5f;
        if (d >= interest->directions[i]) {
            interest->directions[i] = d;
            #if SAC_DEBUG
            interest->entities[i] = param.target;
            priority->directions[i] = 1.0f;
            #endif
        }
    }
#if 0

        if (d >= maxDotProduct) {
            if (d > danger->directions[i]) {
                danger->directions[i] = d;
                #if SAC_DEBUG
                danger->entities[i] = param.target;
                #endif
            }
        } else {
            float iValue = 0;
            if (d > 0) {
                iValue = d;// / maxDotProduct;
            } else {
                iValue = -d;
            }

            if (iValue > interest->directions[i]) {
                interest->directions[i] = iValue;
                #if SAC_DEBUG
                interest->entities[i] = param.target;
                priority->directions[i] = 1.0f;
                #endif
            }
        }
    }
#endif
}

template<>
void behavior(Entity e, float, AvoidParams& param, Context*, Context*, Context* danger) {
    if (param.count == 0)
        return;

    const auto* pc = PHYSICS(e);
    float speed = glm::length(pc->linearVelocity);
    const auto* tc = TRANSFORM(e);
    const float biggestSide = glm::max(tc->size.x, tc->size.y);

    // assume obstacle is not-moving
    // we calculate predicted new pos of e for each direction
    // if we intersect with an object, fill danger map
    const float maxSpeed = 5;
    const float maxForce = 30;

    std::vector<Entity> avoids;
    avoids.reserve(param.count);
    for (int i=0; i<8; i++) {
        avoids.clear();
        // only consider entities within reach
        for (int j=0; j<param.count; j++) {
            Entity a = param.entities[j];
            const auto* t = TRANSFORM(a);
            glm::vec2 diff = t->position - tc->position;
            if (glm::length(diff) <= (glm::max(t->size.x, t->size.y) + biggestSide + maxSpeed * 0.3)) {
                avoids.push_back(a);
            }
        }

        // calculate new pos
        float rotation = Steering::angle(tc->rotation, i);
        glm::vec2 accel = Steering::direction(tc->rotation, i) * maxForce / pc->mass;

        // iterate over a few discrete times in future. danger value depends on when a collision would happen
        bool atLeastOneCollision = true;

        for (int step = 0; step < 10 && atLeastOneCollision; step++) {
            atLeastOneCollision = false;

            float FramePrediction = 0.3 - step * 0.03;
            float dangerValue = step * 0.1;

            // compute size of 'e' upon 2 frame
            // example: [0]---[1] (pos at 0 and 1)
            // result:  [*******]
            const glm::vec2 interpSize = glm::vec2(tc->size.x * (1 + speed * FramePrediction), tc->size.y);


            glm::vec2 movement = (pc->linearVelocity + accel * FramePrediction) * FramePrediction;
            float l = glm::length(movement);
            if (l > maxSpeed) movement *= maxSpeed / l;
            glm::vec2 nextFramePosition = tc->position + movement;
            // Draw::Point(nextFramePosition);
            glm::vec2 interpPos = (tc->position + nextFramePosition) * 0.5f;
            // Draw::Point(interpPos, Color(0, 0, 0));


            int count = avoids.size();
            for (int j=0; j<count; j++) {
                Entity avoid = avoids[j];
                if (!avoid || avoid == e) continue;

                const auto* ta = TRANSFORM(avoid);

                if (IntersectionUtil::rectangleRectangle(interpPos, interpSize, rotation,
                    ta->position, ta->size, ta->rotation)) {
                    atLeastOneCollision = true;

                    if (dangerValue > danger->directions[i]) {
                        danger->directions[i] = dangerValue;
                        #if SAC_DEBUG
                        danger->entities[i] = avoid;
                        #endif
                    }
                    // no need to iterate over every possible obstacle
                    // we only want to know if there's a collision that could occur at t + FramePrediction
                    break;
                } else {
                    // remove entity from  potential collider
                    avoids[j] = 0;
                }
            }
        }
    }
}

template<>
void behavior(Entity e, float, SeparationParams& param, Context* , Context* , Context* danger) {
    return;
    // fill danger map to avoid running into neighbors
    const auto& pos = TRANSFORM(e)->position;
    for (int i=0; i<8; i++) {
        const glm::vec2 direction = Steering::direction(TRANSFORM(e)->rotation, i);
        for (int j=0; j<param.count; j++) {
            glm::vec2 diff = TRANSFORM(param.entities[j])->position - pos;
            float d = glm::dot(diff, direction);
            float dangerValue = (1.0 - d / param.radius) * 0.2;

            if (dangerValue > danger->directions[i]) {
                danger->directions[i] = dangerValue;
                #if SAC_DEBUG
                danger->entities[i] = param.entities[j];
                #endif
            }
        }
    }
}

template<>
void behavior(Entity e, float, AlignmentParams& param, Context* interest, Context* priority, Context*) {
    // return;
    // fill interest to move in the same direction
    float targetAngle = 0;
    for (int j=0; j<param.count; j++) {
        const glm::vec2& v = PHYSICS(param.entities[j])->linearVelocity;
        targetAngle += glm::atan(v.y, v.x);
    }
    targetAngle /= param.count;

    for (int i=0; i<8; i++) {
        float angle = Steering::angle(TRANSFORM(e)->rotation, i);

        float d = targetAngle - angle;
        // maximize cosinus of difference, and minimize sinus
        float c = glm::cos(d);
        float s = glm::abs(glm::sin(d));

        if (s < 0.7) {
            float interestValue = c * 0.5;
            if (interestValue > interest->directions[i]) {
                interest->directions[i] = interestValue;
                priority->directions[i] = 0.3;
            }
        }
    }
}

template<>
void behavior(Entity e, float dt, CohesionParams& param, Context* interest, Context* priority, Context* danger) {
    static Entity eTarget = 0;
    if (!eTarget) {
        eTarget = theEntityManager.CreateEntity(HASH("__/cohesion", 0x6cdf7e4e));
        ADD_COMPONENT(eTarget, Transformation);
        TRANSFORM(eTarget)->size = glm::vec2(0.01f);
    }

    glm::vec2 targetPosition(0.0f);
    for (int j=0; j<param.count; j++) {
        targetPosition += TRANSFORM(param.entities[j])->position;
    }
    targetPosition /= param.count;
    glm::vec2 diff = targetPosition - TRANSFORM(e)->position;
    float length = glm::length(diff);
    float coeff = length / param.radius;
    diff /= length;

    SeekParams seek;
    seek.entities = &eTarget;
    TRANSFORM(eTarget)->position = targetPosition;
    Draw::Point(targetPosition);
    seek.count = 1;
    float weight = 0.3 + 0.7 * coeff;
    seek.weight = &weight;

    behavior(e, dt, seek, interest, priority, danger);
    return;

    for (int i=0; i<8; i++) {
        glm::vec2 dir = Steering::direction(TRANSFORM(e)->rotation, i);
        float d = glm::dot(diff, dir);
        if (d > 0.7) {
            float interestValue = d * 0.7;
            if (interestValue > interest->directions[i]) {
                interest->directions[i] = interestValue;
                priority->directions[i] = 0.3 + 0.7 * coeff;
            }
        }
    }
}

template<>
void behavior(Entity e, float dt, GroupParams& param, Context* interest, Context* priority, Context* danger) {
    std::vector<Entity> neighbors;
    neighbors.reserve(param.count);

    // build neighbor group
    const auto* tc = TRANSFORM(e);
    for (int i=0; i<param.count; i++) {
        if (param.entities[i] == e)
            continue;
        if (glm::distance(tc->position, TRANSFORM(param.entities[i])->position) <= param.neighborRadius) {
            neighbors.push_back(param.entities[i]);
        }
    }

    if (neighbors.empty())
        return;

    // apply separation
    {
        SeparationParams sep;
        sep.entities = &neighbors[0];
        sep.count = (int) neighbors.size();
        sep.radius = param.neighborRadius;
        behavior(e, dt, sep, interest, priority, danger);
    }

    // apply alignment
    {
        AlignmentParams ali;
        ali.entities = &neighbors[0];
        ali.count = (int) neighbors.size();
        ali.radius = param.neighborRadius;
        behavior(e, dt, ali, interest, priority, danger);
    }

    // apply cohesion
    {
        CohesionParams coh;
        coh.entities = &neighbors[0];
        coh.count = (int) neighbors.size();
        coh.radius = param.neighborRadius;
        behavior(e, dt, coh, interest, priority, danger);
    }
}

template<>
void behavior(Entity e, float dt, WanderParams& param, Context* interest, Context* priority, Context* danger) {
    static Entity eTarget = 0;
    if (!eTarget) {
        eTarget = theEntityManager.CreateEntity(HASH("__/wander", 0x7f43116b));
        ADD_COMPONENT(eTarget, Transformation);
        TRANSFORM(eTarget)->size = glm::vec2(0.01f);
    }

    param.target += glm::vec2(
        Random::Float(-1.0f, 1.0f) * param.jitter * dt,
        Random::Float(-1.0f, 1.0f) * param.jitter * dt);
    param.target = glm::normalize(param.target);
    param.target *= param.radius;



    auto* tc = TRANSFORM(eTarget);
    /*if (glm::distance(TRANSFORM(e)->position, param.target) < TRANSFORM(e)->size.x
        || (param.change -= dt) <= 0) {

        float angle = Random::Float(0, 6.28);
        param.target = TRANSFORM(e)->position +
            glm::rotate(glm::vec2(param.radius), angle);
        param.change = param.pauseDuration.random();
    }*/
    tc->position =
        TRANSFORM(e)->position +
        glm::rotate(glm::vec2(param.distance, 0.0f) + param.target, TRANSFORM(e)->rotation);


#if 0
    Draw::Point(tc->position, Color(1, 0, 0));
#endif
    SeekParams seek;
    seek.entities = &eTarget;
    seek.count = 1;
    float weight = 0.25;
    seek.weight = &weight;

    behavior(e, dt, seek, interest, priority, danger);
}


}
