/*
	This file is part of sac.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "AutonomousAgentSystem.h"
#include "../steering/SteeringBehavior.h"
#include "TransformationSystem.h"
#include "PhysicsSystem.h"

INSTANCE_IMPL(AutonomousAgentSystem);

AutonomousAgentSystem::AutonomousAgentSystem() : ComponentSystemImpl<AutonomousAgentComponent>("AutonomousAgent") {
    /* nothing saved */
}

void AutonomousAgentSystem::DoUpdate(float dt) {
	for(std::map<Entity, AutonomousAgentComponent*>::iterator jt=components.begin(); jt!=components.end(); ++jt) {
		Entity e = jt->first;
		AutonomousAgentComponent* agent = (*jt).second;
		Vector2 force(Vector2::Zero);

		if (agent->seekTarget && agent->seekWeight > 0) {
			if (agent->arriveDeceleration > 0) {
				force += SteeringBehavior::arrive(e, TRANSFORM(agent->arriveTarget)->position, agent->maxSpeed, agent->arriveDeceleration) * agent->arriveWeight;
			} else {
				force += SteeringBehavior::seek(e, TRANSFORM(agent->seekTarget)->position, agent->maxSpeed) * agent->seekWeight;
			}
		}
		if (agent->fleeTarget && agent->fleeWeight > 0) {
			if (Vector2::Distance(TRANSFORM(e)->position, TRANSFORM(agent->fleeTarget)->position) < agent->fleeRadius) {
				force += SteeringBehavior::flee(e, TRANSFORM(agent->fleeTarget)->position, agent->maxSpeed) * agent->fleeWeight;
			}
		}
		
		if (agent->wanderWeight > 0) {
			force += SteeringBehavior::wander(e, agent->wander, agent->maxSpeed) * agent->wanderWeight;
		}

		if (force == Vector2::Zero)
			continue;
		float norm = force.Normalize();
		
		PHYSICS(e)->forces.push_back(std::make_pair(Force(force * MathUtil::Min(norm, agent->maxForce), Vector2::Zero), dt));
		
		// orient along movement
		TRANSFORM(e)->rotation = MathUtil::AngleFromVector(PHYSICS(e)->linearVelocity);
	}
}
