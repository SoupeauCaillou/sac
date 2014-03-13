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

INSTANCE_IMPL(AutonomousAgentSystem);

AutonomousAgentSystem::AutonomousAgentSystem() : ComponentSystemImpl<AutonomousAgentComponent>("AutonomousAgent") {
    AutonomousAgentComponent ac;
    componentSerializer.add(new Property<float>("max_speed", OFFSET(maxSpeed, ac), 0.0001f));
    componentSerializer.add(new Property<float>("max_force", OFFSET(maxForce, ac), 0.0001f));
    componentSerializer.add(new Property<float>("flee_weight", OFFSET(fleeWeight, ac), 0.0001f));
    componentSerializer.add(new Property<float>("flee_radius", OFFSET(fleeRadius, ac), 0.0001f));
    componentSerializer.add(new Property<float>("obstacles_weight", OFFSET(obstaclesWeight, ac), 0.0001f));
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

void AutonomousAgentSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(AutonomousAgent, e, agent)
	    LOGF_IF(e == agent->seekTarget, e << ": I can't be my own target!");
	    LOGF_IF(e == agent->fleeTarget, e << ": I can't be my own predator!");
		glm::vec2 force(glm::vec2(0.0f));

		if (agent->seekTarget && agent->seekWeight > 0) {
			if (agent->arriveDeceleration > 0) {
				force += SteeringBehavior::arrive(e, TRANSFORM(agent->arriveTarget)->position, agent->maxSpeed, agent->arriveDeceleration) * agent->arriveWeight;
			} else {
				force += SteeringBehavior::seek(e, TRANSFORM(agent->seekTarget)->position, agent->maxSpeed) * agent->seekWeight;
			}
		}
		if (agent->fleeTarget && agent->fleeWeight > 0) {
			if (glm::distance(TRANSFORM(e)->position, TRANSFORM(agent->fleeTarget)->position) < agent->fleeRadius) {
				force += SteeringBehavior::flee(e, TRANSFORM(agent->fleeTarget)->position, agent->maxSpeed) * agent->fleeWeight;
			}
		}

		if (agent->wanderWeight > 0) {
			force += SteeringBehavior::wander(e, agent->wander, agent->maxSpeed) * agent->wanderWeight;
		}

		if (! agent->obstacles.empty() && agent->obstaclesWeight > 0) {
			force += SteeringBehavior::avoid(e, PHYSICS(e)->linearVelocity, agent->obstacles, agent->maxSpeed) * agent->obstaclesWeight;
		}

        //group behaviors
        if (! agent->cohesionNeighbors.empty() && agent->cohesionWeight > 0) {
            force += SteeringBehavior::groupCohesion(e, agent->cohesionNeighbors, agent->maxSpeed) * agent->cohesionWeight;
        }
        if (! agent->alignementNeighbors.empty() && agent->alignementWeight > 0) {
            force += SteeringBehavior::groupCohesion(e, agent->alignementNeighbors, agent->maxSpeed) * agent->alignementWeight;
        }
        if (! agent->separationNeighbors.empty() && agent->separationWeight > 0) {
            force += SteeringBehavior::groupCohesion(e, agent->separationNeighbors, agent->maxSpeed) * agent->separationWeight;
        }



		if (force == glm::vec2(0.0f))
			continue;
		float norm = glm::length(force);
        force = glm::normalize(force);

		PHYSICS(e)->addForce(force * glm::min(norm, agent->maxForce), glm::vec2(0.f), dt);
	END_FOR_EACH()
}
