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
#include "PhysicsSystem.h"
#include "TransformationSystem.h"

INSTANCE_IMPL(PhysicsSystem);
 
PhysicsSystem::PhysicsSystem() : ComponentSystemImpl<PhysicsComponent>("Physics") {
 
}

void PhysicsSystem::DoUpdate(float dt) {
	//update orphans first
    for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = it->first;
		PhysicsComponent* pc = it->second;
		TransformationComponent* tc = TRANSFORM(a);
		if (!tc || tc->parent == 0) {
			if (pc->mass <= 0)
				continue;
		
			pc->momentOfInertia = pc->mass * tc->size.X * tc->size.Y / 6;
		
			// linear accel
			Vector2 linearAccel(pc->gravity * pc->mass);
			for (unsigned int i=0; i<pc->forces.size(); i++) {
				linearAccel += pc->forces[i].vector;
			}
			linearAccel /= pc->mass;
			// angular accel
			float angAccel = 0;
			for (unsigned int i=0; i<pc->forces.size(); i++) {
		        const Force& force = pc->forces[i];
		        if (force.point != Vector2::Zero) {
			        angAccel += Vector2::Dot(force.point.Perp(), force.vector);
		        }
			}
			angAccel /= pc->momentOfInertia;
			// dumb integration
			pc->linearVelocity += linearAccel * dt;
			tc->position += pc->linearVelocity * dt;
			pc->angularVelocity += angAccel * dt;
			tc->rotation += pc->angularVelocity * dt;
			
			pc->forces.clear();
	    }
	}
    //copy parent property to its sons
    for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = it->first;
		if (!TRANSFORM(a))
			continue;
			
		Entity parent = TRANSFORM(a)->parent;
		if (parent) {
			while (TRANSFORM(parent)->parent) {
				parent = TRANSFORM(parent)->parent;
			}
			PHYSICS(a)->linearVelocity = PHYSICS(parent)->linearVelocity;
			PHYSICS(a)->angularVelocity = PHYSICS(parent)->angularVelocity;
		}
    }
}


void PhysicsSystem::addMoment(PhysicsComponent* pc, float m) {
	// add 2 opposed forces
	pc->forces.push_back(Force(Vector2(0, m * 0.5), Vector2(1, 0)));
	pc->forces.push_back(Force(Vector2(0, -m * 0.5), Vector2(-1, 0)));
}

