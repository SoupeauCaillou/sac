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

INSTANCE_IMPL(AutonomousAgentSystem);

#define EXPORT_BEHAVIOR_PARAM(type) \
    do { \
        componentSerializer.add(new Property<float>( #type "_weight", OFFSET(type##Params.weight, ac), 0.001f)); \
        componentSerializer.add(new Property<float>( #type "_coeff", OFFSET(type##Params.coeff, ac), 0.001f)); \
    } while (false)

AutonomousAgentSystem::AutonomousAgentSystem() : ComponentSystemImpl<AutonomousAgentComponent>("AutonomousAgent") {
    AutonomousAgentComponent ac;
    componentSerializer.add(new Property<float>("max_speed", OFFSET(maxSpeed, ac), 0.0001f));
    componentSerializer.add(new Property<float>("max_force", OFFSET(maxForce, ac), 0.0001f));

    EXPORT_BEHAVIOR_PARAM(flee);
    componentSerializer.add(new Property<float>("flee_radius", OFFSET(fleeRadius, ac), 0.0001f));
    EXPORT_BEHAVIOR_PARAM(obstacles);
    EXPORT_BEHAVIOR_PARAM(walls);
    EXPORT_BEHAVIOR_PARAM(wander);
    componentSerializer.add(new Property<float>("wander_radius", OFFSET(wander.radius, ac), 0.0001f));
    componentSerializer.add(new Property<float>("wander_distance", OFFSET(wander.distance, ac), 0.0001f));
    componentSerializer.add(new Property<float>("wander_jitter", OFFSET(wander.jitter, ac), 0.0001f));
    EXPORT_BEHAVIOR_PARAM(alignement);
    EXPORT_BEHAVIOR_PARAM(separation);
    EXPORT_BEHAVIOR_PARAM(cohesion);
}

bool AutonomousAgentSystem::isArrived(Entity e) {
   return IntersectionUtil::rectangleRectangle(TRANSFORM(e), TRANSFORM(AUTONOMOUS(e)->arriveTarget));
}

// TODO
// - modifier les SteeringBehavior pour qu'ils renvoient la vitesse qu'ils désirent avoir (et dont la norme est <= maxSpeed)
// - modifier AutonomousAgent pour faire la moyenne pondérée de ces vitesse sous la forme:
//        vitesse_moyenne = Somme(poids * (vitesse_desiree - vitesse actuelle)) / somme(poids_des_vitesse_non_nuls)
// - enfin, appliquer une force dont l'amplitude dépend de la vitesse_moyenne et de forceMax
void AutonomousAgentSystem::DoUpdate(float dt) {
#if SAC_DEBUG
    Draw::Clear(__FILE__);
#endif
    FOR_EACH_ENTITY_COMPONENT(AutonomousAgent, e, agent)
        LOGF_IF(e == agent->seekTarget, e << ": I can't be my own target!");
        LOGF_IF(e == agent->fleeTarget, e << ": I can't be my own predator!");
        glm::vec2 force(glm::vec2(0.0f));

        std::vector<std::tuple<float, glm::vec2>> velocities;

        auto* pc = PHYSICS(e);

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
                    Draw::Vec2(__FILE__, TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "arrive");
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
                Draw::Vec2(__FILE__, TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "seek");
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
                    Draw::Vec2(__FILE__, TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "flee");
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
                Draw::Vec2(__FILE__, TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "wander");
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
                Draw::Vec2(__FILE__, TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "obstacle");
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
                Draw::Vec2(__FILE__, TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "wall");
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
                Draw::Vec2(__FILE__, TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "coh");
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
                Draw::Vec2(__FILE__, TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "ali");
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
                Draw::Vec2(__FILE__, TRANSFORM(e)->position, v, Color(0.0, 0, 0.2), "sep");
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
        LOGI_EVERY_N(60, __(glm::length(averageDelta)) << " vs " << __(agent->maxForce));

        PHYSICS(e)->addForce(averageDelta, glm::vec2(0.f), dt);
    END_FOR_EACH()
}
