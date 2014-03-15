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



#pragma once

#include "../steering/SteeringBehavior.h"

#include "System.h"

struct AutonomousAgentComponent {
	AutonomousAgentComponent() : maxSpeed(1.f), maxForce(1.f), seekTarget(0), seekWeight(0.f),
	fleeTarget(0), fleeWeight(0.f), fleeRadius(1.f), obstaclesWeight(0.f), wallsWeight(0.f), wanderWeight(0.f),
	cohesionWeight(0.f), alignementWeight(0.f), separationWeight(0.f) {}

	float maxSpeed, maxForce;
	union {
		Entity seekTarget; // keep speed until target reached
		Entity arriveTarget; // slow down when approching the target
	};
	union {
		float seekWeight;
		float arriveWeight;
	};
	float arriveDeceleration;

	Entity fleeTarget;
	float fleeWeight;
	float fleeRadius;

	std::list<Entity> obstacles;
	float obstaclesWeight;

    std::list<Entity> walls;
    float wallsWeight;

	SteeringBehavior::WanderParams wander;
	float wanderWeight;

	// Group behaviors
	std::list<Entity> cohesionNeighbors;
	float cohesionWeight;

	// Group behaviors
	std::list<Entity> alignementNeighbors;
	float alignementWeight;

	// Group behaviors
	std::list<Entity> separationNeighbors;
	float separationWeight;
};

#define theAutonomousAgentSystem AutonomousAgentSystem::GetInstance()
#if SAC_DEBUG
#define AUTONOMOUS(entity) theAutonomousAgentSystem.Get(entity,true,__FILE__,__LINE__)
#else
#define AUTONOMOUS(entity) theAutonomousAgentSystem.Get(entity)
#endif

UPDATABLE_SYSTEM(AutonomousAgent)
public:
	static bool isArrived(Entity e);

};
