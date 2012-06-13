/*
	This file is part of Heriswap.

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
#include "base/Log.h"

#include "System.h"
#include <vector>

struct Force {
    Vector2 vector;
    Vector2 point;
    
    Force(const Vector2& v, const Vector2& p) : vector(v), point(p) {}
    Force() {}
};

struct PhysicsComponent {
    Vector2 linearVelocity;
    float angularVelocity;
    float mass;
    Vector2 gravity;
    std::vector<Force> forces;
    float momentOfInertia;
};

#define thePhysicsSystem PhysicsSystem::GetInstance()
#define PHYSICS(actor) thePhysicsSystem.Get(actor)
UPDATABLE_SYSTEM(Physics)

public:
	static void addMoment(PhysicsComponent* pc, float m);
};

