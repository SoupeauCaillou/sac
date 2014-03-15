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

AutonomousAgentSystem::AutonomousAgentSystem() : ComponentSystemImpl<AutonomousAgentComponent>("AutonomousAgent") {
    AutonomousAgentComponent ac;
    componentSerializer.add(new Property<float>("max_speed", OFFSET(maxSpeed, ac), 0.0001f));
    componentSerializer.add(new Property<float>("max_force", OFFSET(maxForce, ac), 0.0001f));
    componentSerializer.add(new Property<float>("flee_weight", OFFSET(fleeWeight, ac), 0.0001f));
    componentSerializer.add(new Property<float>("flee_radius", OFFSET(fleeRadius, ac), 0.0001f));
    componentSerializer.add(new Property<float>("obstacles_weight", OFFSET(obstaclesWeight, ac), 0.0001f));
    componentSerializer.add(new Property<float>("walls_weight", OFFSET(wallsWeight, ac), 0.0001f));
    componentSerializer.add(new Property<float>("wander_weight", OFFSET(wanderWeight, ac), 0.0001f));
    componentSerializer.add(new Property<float>("wander_radius", OFFSET(wander.radius, ac), 0.0001f));
    componentSerializer.add(new Property<float>("wander_distance", OFFSET(wander.distance, ac), 0.0001f));
    componentSerializer.add(new Property<float>("wander_jitter", OFFSET(wander.jitter, ac), 0.0001f));


    componentSerializer.add(new Property<float>("alignement_weight", OFFSET(alignementWeight, ac), 0.0001f));
    componentSerializer.add(new Property<float>("separation_weight", OFFSET(separationWeight, ac), 0.0001f));
    componentSerializer.add(new Property<float>("cohesion_weight", OFFSET(cohesionWeight, ac), 0.0001f));
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
    FOR_EACH_ENTITY_COMPONENT(AutonomousAgent, e, agent)
	    LOGF_IF(e == agent->seekTarget, e << ": I can't be my own target!");
	    LOGF_IF(e == agent->fleeTarget, e << ": I can't be my own predator!");
		glm::vec2 force(glm::vec2(0.0f));

        std::vector<std::tuple<float, glm::vec2>> velocities;

#if SAC_DEBUG
        Draw::Clear(__FILE__);
#endif
        auto* pc = PHYSICS(e);
        float length = glm::length(pc->linearVelocity);
        if (length > agent->maxSpeed) {
            pc->linearVelocity *= agent->maxSpeed / length;
        }

		if (agent->seekTarget && agent->seekWeight > 0) {
			if (agent->arriveDeceleration > 0) {
                velocities.push_back(
                    std::make_tuple(
                        agent->arriveWeight,
                        SteeringBehavior::arrive(e, TRANSFORM(agent->arriveTarget)->position, agent->maxSpeed, agent->arriveDeceleration)
                    ));
#if SAC_DEBUG
                Draw::Vec2(__FILE__, TRANSFORM(e)->position, std::get<1>(velocities.back()), Color(0.9, 1, 1), "arrive");
#endif
			} else {
                velocities.push_back(
                    std::make_tuple(
                        agent->seekWeight,
                        SteeringBehavior::seek(e, TRANSFORM(agent->seekTarget)->position, agent->maxSpeed)
                    ));
#if SAC_DEBUG
                Draw::Vec2(__FILE__, TRANSFORM(e)->position, std::get<1>(velocities.back()), Color(0.9, 1, 1), "seek");
#endif
			}
		}
		if (agent->fleeTarget && agent->fleeWeight > 0) {
			if (glm::distance(TRANSFORM(e)->position, TRANSFORM(agent->fleeTarget)->position) < agent->fleeRadius) {
                velocities.push_back(
                    std::make_tuple(
                        agent->fleeWeight,
                        SteeringBehavior::flee(e, TRANSFORM(agent->fleeTarget)->position, agent->maxSpeed)
                    ));

#if SAC_DEBUG
                Draw::Vec2(__FILE__, TRANSFORM(e)->position, std::get<1>(velocities.back()), Color(0.9, 1, 1), "flee");
#endif
			}
		}

		if (agent->wanderWeight > 0) {
            velocities.push_back(
                std::make_tuple(
                    agent->wanderWeight,
                    SteeringBehavior::wander(e, agent->wander, agent->maxSpeed)
                ));
#if SAC_DEBUG
            Draw::Vec2(__FILE__, TRANSFORM(e)->position, std::get<1>(velocities.back()), Color(0.9, 1, 1), "wander");
#endif
		}

		if (! agent->obstacles.empty() && agent->obstaclesWeight > 0) {
            velocities.push_back(
                std::make_tuple(
                    agent->obstaclesWeight,
                    SteeringBehavior::obstacleAvoidance(e, pc->linearVelocity, agent->obstacles, agent->maxSpeed)
                ));
#if SAC_DEBUG
            Draw::Vec2(__FILE__, TRANSFORM(e)->position, std::get<1>(velocities.back()), Color(0.9, 1, 1), "obstacle");
#endif
        }

        if (! agent->walls.empty() && agent->wallsWeight > 0) {
            velocities.push_back(
                std::make_tuple(
                    agent->wallsWeight,
                    SteeringBehavior::wallAvoidance(e, pc->linearVelocity, agent->walls, agent->maxSpeed)
                ));
#if SAC_DEBUG
            Draw::Vec2(__FILE__, TRANSFORM(e)->position, std::get<1>(velocities.back()), Color(0.9, 1, 1), "wall");
#endif
        }

        //group behaviors
        if (! agent->cohesionNeighbors.empty() && agent->cohesionWeight > 0) {
            velocities.push_back(
                std::make_tuple(
                    agent->cohesionWeight,
                    SteeringBehavior::groupCohesion(e, agent->cohesionNeighbors, agent->maxSpeed)
                ));
#if SAC_DEBUG
            Draw::Vec2(__FILE__, TRANSFORM(e)->position, std::get<1>(velocities.back()), Color(0.9, 1, 1), "cohesion");
#endif
        }
        if (! agent->alignementNeighbors.empty() && agent->alignementWeight > 0) {
            velocities.push_back(
                std::make_tuple(
                    agent->alignementWeight,
                    SteeringBehavior::groupAlign(e, agent->alignementNeighbors, agent->maxSpeed)
                ));
#if SAC_DEBUG
            Draw::Vec2(__FILE__, TRANSFORM(e)->position, std::get<1>(velocities.back()), Color(0.9, 1, 1), "alignement");
#endif
        }
        if (! agent->separationNeighbors.empty() && agent->separationWeight > 0) {
            velocities.push_back(
                std::make_tuple(
                    agent->separationWeight,
                    SteeringBehavior::groupSeparate(e, agent->separationNeighbors, agent->maxSpeed)
                ));
#if SAC_DEBUG
            Draw::Vec2(__FILE__, TRANSFORM(e)->position, std::get<1>(velocities.back()), Color(0.9, 1, 1), "separation");
#endif
        }

        if (velocities.empty()) {
            continue;
        }

        // Compute weighted average of: desired_velocity - current_velocity
        glm::vec2 averageDelta(0.0f);
        float sumWeight = 0;
        for (const auto& wv: velocities) {
            const auto& v = std::get<1>(wv);
            if (glm::length2(v) > 0) {
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
