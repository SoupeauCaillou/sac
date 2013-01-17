#include "AutonomousAgentSystem.h"

#include <base/Assert.h>

#include "base/MathUtil.h"
#include "TransformationSystem.h"
#include "PhysicsSystem.h"

INSTANCE_IMPL(AutonomousAgentSystem);

AutonomousAgentSystem::AutonomousAgentSystem() : ComponentSystemImpl<AutonomousAgentComponent>("AutonomousAgent") {
    /* nothing saved */
}

void AutonomousAgentSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(AutonomousAgent, e, agent)
		ASSERT(e != agent->seekTarget, e << ": I can't be my own target!");
		ASSERT(e != agent->fleeTarget, e << ": I can't be my own predator!");

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
