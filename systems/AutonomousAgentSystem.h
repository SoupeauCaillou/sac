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
#pragma once

#include "base/MathUtil.h"
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

