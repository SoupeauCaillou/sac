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

#if !DISABLE_AUTONOMOUS_SYSTEM

#include "AutonomousAgentSystem.h"

#include "../steering/SteeringBehavior.h"
#include "TransformationSystem.h"
#include "PhysicsSystem.h"
#include "util/IntersectionUtil.h"
#include "base/Log.h"

#include <glm/gtx/norm.hpp>
#include "util/SerializerProperty.h"

#if SAC_DEBUG
#include "util/Draw.h"
#endif

#include "../steering/SimpleBehavior.inl"
#include <algorithm>

INSTANCE_IMPL(AutonomousAgentSystem);

#define EXPORT_BEHAVIOR_PARAM(type, hash_w, hash_c) \
    do { \
        componentSerializer.add(new Property<float>(HASH( #type "_weight", hash_w), OFFSET(type##Params.weight, ac), 0.001f)); \
        componentSerializer.add(new Property<float>(HASH( #type "_coeff", hash_c), OFFSET(type##Params.coeff, ac), 0.001f)); \
    } while (false)

AutonomousAgentSystem::AutonomousAgentSystem() : ComponentSystemImpl<AutonomousAgentComponent>(HASH("AutonomousAgent", 0x63018d2f)) {
    for (int i=0; i<8; i++) {
        glm::vec2 r = glm::rotate(glm::vec2(1, 0), Steering::angle(0, i));
        LOGI("glm::vec2(" << r.x << ", " << r.y << ")");
    }
#if 0

    AutonomousAgentComponent ac;
    componentSerializer.add(new Property<float>(HASH("max_speed", 0x3fbe6552), OFFSET(maxSpeed, ac), 0.0001f));
    componentSerializer.add(new Property<float>(HASH("max_force", 0xe2724098), OFFSET(maxForce, ac), 0.0001f));

    EXPORT_BEHAVIOR_PARAM(flee, 0x5c11f047, 0xaee1e8a1);
    componentSerializer.add(new Property<float>(HASH("flee_radius", 0xf386c18f), OFFSET(fleeRadius, ac), 0.0001f));
    EXPORT_BEHAVIOR_PARAM(obstacles, 0x1a6068b3, 0x5ae2296a);
    EXPORT_BEHAVIOR_PARAM(walls, 0xf5993180, 0xa756e67e);
    EXPORT_BEHAVIOR_PARAM(box, 0x80280d35, 0x884321db);
    componentSerializer.add(new Property<glm::vec2>(HASH("box_position", 0x13c1e7a8), OFFSET(boxPosition, ac), glm::vec2(0.0001f)));
    componentSerializer.add(new Property<glm::vec2>(HASH("box_size", 0x7f33bd9), OFFSET(boxSize, ac), glm::vec2(0.0001f)));
#if 0
    EXPORT_BEHAVIOR_PARAM(wander, 0x1d51e9bc, 0x275e6c94);
    componentSerializer.add(new Property<float>(HASH("wander_radius", 0xd0e7bb52), OFFSET(wander.radius, ac), 0.0001f));
    componentSerializer.add(new Property<float>(HASH("wander_distance", 0xd4de58b5), OFFSET(wander.distance, ac), 0.0001f));
    componentSerializer.add(new Property<float>(HASH("wander_jitter", 0x267a8a75), OFFSET(wander.jitter, ac), 0.0001f));
#endif
    EXPORT_BEHAVIOR_PARAM(alignement, 0xfc9cfd08, 0x1fc5142f);
    EXPORT_BEHAVIOR_PARAM(separation, 0xe6642c0, 0xb6ca78ed);
    EXPORT_BEHAVIOR_PARAM(cohesion, 0x60651d29, 0x624454d6);
#endif
}

bool AutonomousAgentSystem::isArrived(Entity ) {
   return false;//return IntersectionUtil::rectangleRectangle(TRANSFORM(e), TRANSFORM(AUTONOMOUS(e)->arriveTarget));
}

void AutonomousAgentSystem::DoUpdate(float dt) {
    Draw::Clear(HASH("aa", 0x6e1cb412));
    FOR_EACH_ENTITY_COMPONENT(AutonomousAgent, e, agent)
        Steering::Context interest, priority, danger;
        memset(&interest, 0, sizeof(Steering::Context));
        memset(&priority, 0, sizeof(Steering::Context));
        memset(&danger, 0, sizeof(Steering::Context));

        const float rotation = TRANSFORM(e)->rotation;

        /**********
             Individual behaviors
        */
        if (agent->seek.count) {
            Steering::behavior(e, dt, agent->seek, &interest, &priority, &danger);
        }

        if (agent->flee.target) {
            Steering::behavior(e, dt, agent->flee, &interest, &priority, &danger);
        }

        if (agent->avoid.count) {
            Steering::behavior(e, dt, agent->avoid, &interest, &priority, &danger);
        }

        if (agent->group.count) {
            Steering::behavior(e, dt, agent->group, &interest, &priority, &danger);
        }

        if (agent->wander.radius > 0) {
            Steering::behavior(e, dt, agent->wander, &interest, &priority, &danger);
        }

        /**********
             Decision algorithm
        */
        // simple decision algorithm for now.
        float min = agent->dangerThreshold;

        // pick lowest danger direction(s)
        std::vector<int> potentialDirections;
        for (int i=0; i<9; i++) {
            if (danger.directions[i] <= min && interest.directions[i] > danger.directions[i]) {
                potentialDirections.push_back(i);
            }
        }

        std::sort(potentialDirections.begin(), potentialDirections.end(), [&interest, &danger, rotation] (int direction1, int d2) -> bool {
            if (direction1 == d2)
                return false;
            if (direction1 == 4)
                return false;
            if (d2 == 4)
                return true;

            float diff1 = interest.directions[direction1] - danger.directions[direction1];
            float diff2 = interest.directions[d2] - danger.directions[d2];
            if (glm::abs(diff1 - diff2) > 0.01) {
                if (diff1 > diff2)
                    return true;
                if (diff1 < diff2)
                    return false;
            }

            if (direction1 == 8)
                return true;
            if (d2 == 8)
                return false;
            /* Return closest to current orientation */
            return (d2 > direction1) && d2 < (8 - direction1);
        });

        // if no direction available, stop

        int chosenDirection = -1;


        bool cancelVelocity = false;
        if (potentialDirections.empty()) {
            cancelVelocity = true;
        } else {
            chosenDirection = potentialDirections[0];

            if (chosenDirection == 8) {
                // LOGI("STOP HERE PLEASE - " << theEntityManager.entityName(e) << ':' << interest.directions[8]);
                cancelVelocity = true;
            }
        }

        if (!cancelVelocity && priority.directions[chosenDirection] == 0) {
            cancelVelocity = true;
        }
        if (cancelVelocity) {
            PHYSICS(e)->linearVelocity = glm::vec2(0.0f);
            #if 0
            Draw::Point(HASH("aa", 0x6e1cb412), TRANSFORM(e)->position, Color(1, 0, 0, 1));
            #endif

            // add rotating force
            if (potentialDirections.size() <= 1) {
                TRANSFORM(e)->rotation += Random::Float(.0f, 3.0f) * dt;
            }
            /*PHYSICS(e)->addForce(
                glm::vec2(0.0f, 1.0f),
                TRANSFORM(e)->size * 0.5f,
                dt);
            PHYSICS(e)->addForce(
                glm::vec2(0.0f, -1.0f),
                TRANSFORM(e)->size * -0.5f,
                dt);
            PHYSICS(e)->instantRotation = false;*/
        } else {
            PHYSICS(e)->instantRotation = true;
            glm::vec2 forceDirection = Steering::direction(rotation, chosenDirection);

            // lateral speed
            const auto& v = PHYSICS(e)->linearVelocity;
            float t = glm::dot(v, forceDirection);

            glm::vec2 desiredVelocity = forceDirection * agent->maxSpeed;
            glm::vec2 lateralVelocity = v - t * forceDirection;
            glm::vec2 force = glm::normalize(desiredVelocity - lateralVelocity);


            PHYSICS(e)->maxSpeed = priority.directions[chosenDirection] * agent->maxSpeed;
            PHYSICS(e)->addForce(
                force * agent->maxForce,
                glm::vec2(0.f), dt);

            #if 0
            Draw::Point(HASH("aa", 0x6e1cb412), TRANSFORM(e)->position + force, Color(0.5, 0.5, .4, 1));
            #endif
        }

        #if 0
        const glm::vec2& velocity = glm::normalize(PHYSICS(e)->linearVelocity);
        for (int i=0; i<8; i++) {
            if (danger.directions[i])
            {
                glm::vec2 size = Steering::direction(rotation, i) * danger.directions[i];
                Draw::Vec2(HASH("aa", 0x6e1cb412), TRANSFORM(e)->position, size, Color(1, 0, 0, 0.5));
                #if SAC_DEBUG
                if (danger.entities[i]) {
                    glm::vec2 pos = TRANSFORM(e)->position + size;
                    Draw::Vec2(HASH("aa", 0x6e1cb412),
                        pos, TRANSFORM(danger.entities[i])->position - pos, Color(0,0,0, 0.2));
                } else {
                    Draw::Vec2(HASH("aa", 0x6e1cb412),
                        TRANSFORM(e)->position + size, glm::vec2(1, 0), Color(0,0,1, 0.2));
                }
                #endif
            }

            if (interest.directions[i])
            {
                glm::vec2 size = Steering::direction(rotation, i) * interest.directions[i];
                Draw::Vec2(HASH("aa", 0x6e1cb412), TRANSFORM(e)->position, Steering::direction(rotation, i) * interest.directions[i], Color(0, 1, 0, 1));
                if (i == chosenDirection) {
                    Draw::Vec2(HASH("aa", 0x6e1cb412),
                        TRANSFORM(e)->position,
                        Steering::direction(rotation, i),
                        Color(0, 0, 0, 1));
                }
                #if SAC_DEBUG
                if (interest.entities[i]) {
                    glm::vec2 pos = TRANSFORM(e)->position + size;
                    Draw::Vec2(HASH("aa", 0x6e1cb412),
                        pos, TRANSFORM(interest.entities[i])->position - pos, Color(0.5,.5,.5, 0.8));
                }
                #endif
            }
        }
        #endif
    }
}
#endif
