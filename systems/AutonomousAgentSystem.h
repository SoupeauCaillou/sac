#pragma once

#include "../steering/SteeringBehavior.h"

#include "System.h"

struct AutonomousAgentComponent {
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
#define AUTONOMOUS_AGENT(entity) theAutonomousAgentSystem.Get(entity)

UPDATABLE_SYSTEM(AutonomousAgent)

};

