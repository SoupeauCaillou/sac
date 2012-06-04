#include "PhysicsSystem.h"
#include "TransformationSystem.h"

INSTANCE_IMPL(PhysicsSystem);
 
PhysicsSystem::PhysicsSystem() : ComponentSystemImpl<PhysicsComponent>("Physics") {
 
}

void PhysicsSystem::DoUpdate(float dt) {
    for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
        Entity a = (*it).first;
        PhysicsComponent* pc = (*it).second;
        TransformationComponent* tc = TRANSFORM(a);
        
        if (pc->mass <= 0)
        	continue;

        pc->momentOfInertia = pc->mass * tc->size.X * tc->size.Y / 6;

        // linear accel
        Vector2 linearAccel(pc->gravity);
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

void PhysicsSystem::addMoment(PhysicsComponent* pc, float m) {
	// add 2 opposed forces
	pc->forces.push_back(Force(Vector2(0, m * 0.5), Vector2(1, 0)));
	pc->forces.push_back(Force(Vector2(0, -m * 0.5), Vector2(-1, 0)));
}

