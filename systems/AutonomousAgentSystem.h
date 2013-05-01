#pragma once

#include "../steering/SteeringBehavior.h"

#include "System.h"

struct AutonomousAgentComponent {
	AutonomousAgentComponent() : maxSpeed(1.), maxForce(1.), seekTarget(0), fleeTarget(0) {}

	float maxSpeed, maxForce;
	union {
		Entity seekTarget;
		Entity arriveTarget;
	};
	union {
		float seekWeight;
		float arriveWeight;
	};
	float arriveDeceleration;

	Entity fleeTarget;
	float fleeWeight;
	float fleeRadius;

	SteeringBehavior::WanderParams wander;
	float wanderWeight;
};

#define theAutonomousAgentSystem AutonomousAgentSystem::GetInstance()
#define AUTONOMOUS(entity) theAutonomousAgentSystem.Get(entity)

UPDATABLE_SYSTEM(AutonomousAgent)
public:
	static bool isArrived(Entity e);

};
