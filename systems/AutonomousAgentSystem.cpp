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

#if SAC_DEBUG
#include "util/Draw.h"
#endif

#include "../steering/SimpleBehavior.inl"


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
        if (agent->seek.target) {
            Steering::behavior(e, agent->seek, &interest, &priority, &danger);
        }

        if (agent->flee.target) {
            Steering::behavior(e, agent->flee, &interest, &priority, &danger);
        }

        if (agent->avoid.count) {
            Steering::behavior(e, agent->avoid, &interest, &priority, &danger);
        }

        if (agent->group.count) {
            Steering::behavior(e, agent->group, &interest, &priority, &danger);
        }

        /**********
             Decision algorithm
        */
        // simple decision algorithm for now.
        float min = agent->dangerThreshold;

        // pick lowest danger direction(s)
        std::vector<int> potentialDirections;
        for (int i=0; i<8; i++) {
            if (danger.directions[i] <= min) {
                potentialDirections.push_back(i);
            }
        }

        float max = 0;
        float maxDiff = 0.0f;

        std::sort(potentialDirections.begin(), potentialDirections.end(), [&interest, &danger, rotation] (int direction1, int d2) -> bool {
            if (direction1 == d2)
                return false;

            float diff1 = interest.directions[direction1] - danger.directions[direction1];
            float diff2 = interest.directions[d2] - danger.directions[d2];
            if (glm::abs(diff1 - diff2) > 0.2) {
                if (diff1 > diff2)
                    return true;
                if (diff1 < diff2)
                    return false;
            }
            /* Return closest to current orientation */
            return (d2 > direction1) && d2 < (8 - direction1);
        });

        // if no direction available, stop

        int chosenDirection = -1;
        #if 1
        const glm::vec2& velocity = glm::normalize(PHYSICS(e)->linearVelocity);
        for (int i=0; i<8; i++) {
            glm::vec2 size = Steering::direction(rotation, i) * danger.directions[i] * 2.0f;
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

            float d = glm::max(0.0f, glm::dot(Steering::direction(rotation, i), velocity));
            Draw::Vec2(HASH("aa", 0x6e1cb412), TRANSFORM(e)->position, Steering::direction(rotation, i) * interest.directions[i] * 2.0f, Color(0, 1, d, 0.5));
        }
        #endif

        bool cancelVelocity = false;
        if (potentialDirections.empty()) {
            cancelVelocity = true;
        } else {
            chosenDirection = potentialDirections[0];
        }

        if (!cancelVelocity && priority.directions[chosenDirection] == 0) {
            cancelVelocity = true;
        }
        if (cancelVelocity) {
            LOGI_EVERY_N(60, "No sensible direction choice, stop here");
            float v = glm::length(PHYSICS(e)->linearVelocity);
            PHYSICS(e)->linearVelocity = glm::vec2(0.0f);
            // addForce( PHYSICS(e)->linearVelocity * (-glm::min(agent->maxForce, 3 * v)), glm::vec2(0.f), dt);
            Draw::Point(HASH("aa", 0x6e1cb412), TRANSFORM(e)->position, Color(1, 0, 0, 0.5));

        } else {
            Draw::Point(HASH("aa", 0x6e1cb412), TRANSFORM(e)->position
                + Steering::direction(rotation, chosenDirection) * interest.directions[chosenDirection], Color(0, 0, 1, 0.5));
            PHYSICS(e)->addForce(Steering::direction(rotation, chosenDirection) * ( priority.directions[chosenDirection] * agent->maxForce), glm::vec2(0.f), dt);
        }

        // PHYSICS(e)->linearVelocity = Steering::direction(rotation, chosenDirection) * ( priority.directions[chosenDirection] * 5);
    }
#if 0
        LOGF_IF(e == agent->seekTarget, e << ": I can't be my own target!");
        LOGF_IF(e == agent->fleeTarget, e << ": I can't be my own predator!");
        glm::vec2 force(glm::vec2(0.0f));

        std::vector<std::tuple<float, glm::vec2>> velocities;

        auto* pc = PHYSICS(e);

        if (pc->mass <= 0)
            continue;

        if (glm::abs(glm::length(pc->linearVelocity)) > 0) {
            TRANSFORM(e)->rotation = glm::atan(pc->linearVelocity.y, pc->linearVelocity.x);
        }

        if (agent->seekTarget && agent->seekParams.weight > 0) {
            if (agent->arriveDeceleration > 0) {
                velocities.push_back(
                    std::make_tuple(
                        agent->arriveParams.weight,
                        SteeringBehavior::arrive(e, TRANSFORM(agent->arriveTarget)->position, agent->arriveParams.coeff * agent->maxSpeed, agent->arriveDeceleration)
                    ));
#if SAC_DEBUG
                const auto& v = std::get<1>(velocities.back());
                if (glm::length2(v - pc->linearVelocity) > 0.001) {
                    Draw::Vec2(TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "arrive");
                }
#endif
            } else {
                velocities.push_back(
                    std::make_tuple(
                        agent->seekParams.weight,
                        SteeringBehavior::seek(e, TRANSFORM(agent->seekTarget)->position, agent->seekParams.coeff * agent->maxSpeed)
                    ));
#if SAC_DEBUG
            const auto& v = std::get<1>(velocities.back());
            if (glm::length2(v - pc->linearVelocity) > 0.001) {
                Draw::Vec2(TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "seek");
            }
#endif
            }
        }
        if (agent->fleeTarget && agent->fleeParams.weight > 0) {
            if (glm::distance(TRANSFORM(e)->position, TRANSFORM(agent->fleeTarget)->position) < agent->fleeRadius) {
                velocities.push_back(
                    std::make_tuple(
                        agent->fleeParams.weight,
                        SteeringBehavior::flee(e, TRANSFORM(agent->fleeTarget)->position, agent->fleeParams.coeff * agent->maxSpeed)
                    ));

#if SAC_DEBUG
                const auto& v = std::get<1>(velocities.back());
                if (glm::length2(v - pc->linearVelocity) > 0.001) {
                    Draw::Vec2(TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "flee");
                }
#endif
            }
        }

        if (agent->wanderParams.weight > 0) {
            velocities.push_back(
                std::make_tuple(
                    agent->wanderParams.weight,
                    SteeringBehavior::wander(e, agent->wander, agent->wanderParams.coeff * agent->maxSpeed)
                ));
#if SAC_DEBUG
            const auto& v = std::get<1>(velocities.back());
            if (glm::length2(v - pc->linearVelocity) > 0.001) {
                Draw::Vec2(TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "wander");
            }
#endif
        }

        if (! agent->obstacles.empty() && agent->obstaclesParams.weight > 0) {
            velocities.push_back(
                std::make_tuple(
                    agent->obstaclesParams.weight,
                    SteeringBehavior::obstacleAvoidance(e, pc->linearVelocity, agent->obstacles, agent->obstaclesParams.coeff * agent->maxSpeed)
                ));
#if SAC_DEBUG
            const auto& v = std::get<1>(velocities.back());
            if (glm::length2(v - pc->linearVelocity) > 0.001) {
                Draw::Vec2(TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "obstacle");
            }
#endif
        }

        if (! agent->walls.empty() && agent->wallsParams.weight > 0) {
            velocities.push_back(
                std::make_tuple(
                    agent->wallsParams.weight,
                    SteeringBehavior::wallAvoidance(e, pc->linearVelocity, agent->walls, agent->wallsParams.coeff * agent->maxSpeed)
                ));
#if SAC_DEBUG
            const auto& v = std::get<1>(velocities.back());
            if (glm::length2(v - pc->linearVelocity) > 0.001) {
                Draw::Vec2(TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "wall");
            }
#endif
        }

        if (agent->boxParams.weight > 0) {
            velocities.push_back(
                std::make_tuple(
                    agent->boxParams.weight,
                    SteeringBehavior::boxContainer(e, pc->linearVelocity, agent->boxPosition, agent->boxSize, agent->boxParams.coeff * agent->maxSpeed)
                ));
#if SAC_DEBUG
            const auto& v = std::get<1>(velocities.back());
            if (glm::length2(v - pc->linearVelocity) > 0.001) {
                Draw::Vec2(TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "box");
            }
#endif
        }


        //group behaviors
        if (! agent->cohesionNeighbors.empty() && agent->cohesionParams.weight > 0) {
            velocities.push_back(
                std::make_tuple(
                    agent->cohesionParams.weight,
                    SteeringBehavior::groupCohesion(e, agent->cohesionNeighbors, agent->cohesionParams.coeff * agent->maxSpeed)
                ));
#if SAC_DEBUG
            const auto& v = std::get<1>(velocities.back());
            if (glm::length2(v - pc->linearVelocity) > 0.001) {
                Draw::Vec2(TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "coh");
            }
#endif
        }
        if (! agent->alignementNeighbors.empty() && agent->alignementParams.weight > 0) {
            velocities.push_back(
                std::make_tuple(
                    agent->alignementParams.weight,
                    SteeringBehavior::groupAlign(e, agent->alignementNeighbors, agent->alignementParams.coeff * agent->maxSpeed)
                ));
#if SAC_DEBUG
            const auto& v = std::get<1>(velocities.back());
            if (glm::length2(v - pc->linearVelocity) > 0.001) {
                Draw::Vec2(TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "ali");
            }
#endif
        }
        if (! agent->separationNeighbors.empty() && agent->separationParams.weight > 0) {
            velocities.push_back(
                std::make_tuple(
                    agent->separationParams.weight,
                    SteeringBehavior::groupSeparate(e, agent->separationNeighbors, agent->separationParams.coeff * agent->maxSpeed)
                ));
#if SAC_DEBUG
            const auto& v = std::get<1>(velocities.back());
            if (glm::length2(v - pc->linearVelocity) > 0.001) {
                Draw::Vec2(TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "sep");
            }
#endif
        }

        if (velocities.empty()) {
            continue;
        }

        // Compute weighted average of: desired_velocity - current_velocity
        glm::vec2 averageDelta(0.0f);
        float sumWeight = 0;
        for (const auto& wv: velocities) {
            const auto& v = std::get<1>(wv) - pc->linearVelocity;
            if (glm::length2(v) > 0.0001) {
                averageDelta += std::get<0>(wv) * v;
                sumWeight += std::get<0>(wv);
            }
        }

        if (sumWeight <= 0) {
            continue;
        }

        // Weights are used only to prioritize steering behavior - so we now cancel
        // them from averageDelta
        averageDelta /= sumWeight;

        float norm = glm::length(averageDelta);

        averageDelta /= norm;

        norm = agent->maxForce * norm / agent->maxSpeed;
        averageDelta *= norm;

        if (norm > agent->maxForce) {
            averageDelta *= agent->maxForce / norm;
        }
        LOGI_EVERY_N(6000, __(glm::length(averageDelta)) << " vs " << __(agent->maxForce));

        PHYSICS(e)->addForce(averageDelta, glm::vec2(0.f), dt);
    END_FOR_EACH()
#endif
}
#endif
