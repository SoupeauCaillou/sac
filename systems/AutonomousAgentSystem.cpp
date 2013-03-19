#include "AutonomousAgentSystem.h"

#include <base/Assert.h>

#include "../steering/SteeringBehavior.h"
#include "TransformationSystem.h"
#include "PhysicsSystem.h"
#include "util/IntersectionUtil.h"

INSTANCE_IMPL(AutonomousAgentSystem);

AutonomousAgentSystem::AutonomousAgentSystem() : ComponentSystemImpl<AutonomousAgentComponent>("AutonomousAgent") {
    /* nothing saved */
}

bool AutonomousAgentSystem::isArrived(Entity e) {
   return IntersectionUtil::rectangleRectangle(TRANSFORM(e), TRANSFORM(AUTONOMOUS(e)->arriveTarget));
}

void AutonomousAgentSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(AutonomousAgent, e, agent)
	    ASSERT(e != agent->seekTarget, e << ": I can't be my own target!");
	    ASSERT(e != agent->fleeTarget, e << ": I can't be my own predator!");
		Vector2 force(Vector2::Zero);

		if (agent->seekTarget && agent->seekWeight > 0) {
			if (agent->arriveDeceleration > 0) {
				force += SteeringBehavior::arrive(e, TRANSFORM(agent->arriveTarget)->worldPosition, agent->maxSpeed, agent->arriveDeceleration) * agent->arriveWeight;
			} else {
				force += SteeringBehavior::seek(e, TRANSFORM(agent->seekTarget)->worldPosition, agent->maxSpeed) * agent->seekWeight;
			}
		}
		if (agent->fleeTarget && agent->fleeWeight > 0) {
			if (Vector2::Distance(TRANSFORM(e)->worldPosition, TRANSFORM(agent->fleeTarget)->worldPosition) < agent->fleeRadius) {
				force += SteeringBehavior::flee(e, TRANSFORM(agent->fleeTarget)->worldPosition, agent->maxSpeed) * agent->fleeWeight;
			}
		}

		if (agent->wanderWeight > 0) {
			force += SteeringBehavior::wander(e, agent->wander, agent->maxSpeed) * agent->wanderWeight;
		}

		if (force == Vector2::Zero)
			continue;
		float norm = force.Normalize();

		PHYSICS(e)->forces.push_back(std::make_pair(Force(force * MathUtil::Min(norm, agent->maxForce), Vector2::Zero), dt));
	}
}

#ifdef INGAME_EDITORS
void AutonomousAgentSystem::addEntityPropertiesToBar(Entity e, TwBar* bar) {

}
#endif

