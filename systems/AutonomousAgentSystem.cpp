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
    componentSerializer.add(new Property<float>("wander_weight", OFFSET(wanderWeight, ac), 0.0001f));
    componentSerializer.add(new Property<float>("wander_radius", OFFSET(wander.radius, ac), 0.0001f));
    componentSerializer.add(new Property<float>("wander_distance", OFFSET(wander.distance, ac), 0.0001f));
    componentSerializer.add(new Property<float>("wander_jitter", OFFSET(wander.jitter, ac), 0.0001f));
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
				force += SteeringBehavior::arrive(e, TRANSFORM(agent->arriveTarget)->worldPosition, agent->maxSpeed, agent->arriveDeceleration) * agent->arriveWeight;
			} else {
				force += SteeringBehavior::seek(e, TRANSFORM(agent->seekTarget)->worldPosition, agent->maxSpeed) * agent->seekWeight;
			}
		}
		if (agent->fleeTarget && agent->fleeWeight > 0) {
			if (glm::distance(TRANSFORM(e)->worldPosition, TRANSFORM(agent->fleeTarget)->worldPosition) < agent->fleeRadius) {
				force += SteeringBehavior::flee(e, TRANSFORM(agent->fleeTarget)->worldPosition, agent->maxSpeed) * agent->fleeWeight;
			}
		}

		if (agent->wanderWeight > 0) {
			force += SteeringBehavior::wander(e, agent->wander, agent->maxSpeed) * agent->wanderWeight;
		}

		if (force == glm::vec2(0.0f))
			continue;
		float norm = glm::length(force);
        force = glm::normalize(force);

		PHYSICS(e)->addForce(glm::vec2(0.f), force * glm::min(norm, agent->maxForce), dt);
	}
}

#if SAC_INGAME_EDITORS
void AutonomousAgentSystem::addEntityPropertiesToBar(Entity, TwBar*) {

}
#endif
