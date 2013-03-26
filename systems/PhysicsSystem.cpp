#include "PhysicsSystem.h"
#include "TransformationSystem.h"

INSTANCE_IMPL(PhysicsSystem);

PhysicsSystem::PhysicsSystem() : ComponentSystemImpl<PhysicsComponent>("Physics") {
    PhysicsComponent tc;
    componentSerializer.add(new Property<Vector2>(OFFSET(linearVelocity, tc), Vector2(0.001, 0)));
    componentSerializer.add(new Property<float>(OFFSET(angularVelocity, tc), 0.001));
    componentSerializer.add(new Property<float>(OFFSET(mass, tc), 0.001));
    componentSerializer.add(new Property<Vector2>(OFFSET(gravity, tc), Vector2(0.001, 0)));
    componentSerializer.add(new Property<float>(OFFSET(momentOfInertia, tc), 0.001));
}

void PhysicsSystem::DoUpdate(float dt) {
	//update orphans first
    FOR_EACH_ENTITY_COMPONENT(Physics, a, pc)
		TransformationComponent* tc = TRANSFORM(a);
		if (!tc || tc->parent == 0) {
			if (pc->mass <= 0)
				continue;

			pc->momentOfInertia = pc->mass * tc->size.X * tc->size.Y / 6;

			// linear accel
			Vector2 linearAccel(pc->gravity * pc->mass);
			// angular accel
			float angAccel = 0;

			for (unsigned int i=0; i<pc->forces.size(); i++) {
				Force force(pc->forces[i].first);

				float& durationLeft = pc->forces[i].second;

				if (durationLeft < dt) {
					force.vector *= durationLeft / dt;
				}

				linearAccel += force.vector;

		        if (force.point != Vector2::Zero) {
			        angAccel += Vector2::Dot(force.point.Perp(), force.vector);
		        }

				durationLeft -= dt;
				if (durationLeft < 0) {
					pc->forces.erase(pc->forces.begin() + i);
					i--;
				}
			}
			linearAccel /= pc->mass;
			angAccel /= pc->momentOfInertia;

			// acceleration is constant over dt: use basic Euler integration for velocity
            const Vector2 nextVelocity(pc->linearVelocity + linearAccel * dt);
            tc->position += (pc->linearVelocity + nextVelocity) * dt * 0.5;
            // velocity varies over dt: use Verlet integration for position
            pc->linearVelocity = nextVelocity;
			const float nextAngularVelocity = pc->angularVelocity + angAccel * dt;
			tc->rotation += (pc->angularVelocity + nextAngularVelocity) * dt * 0.5;
            pc->angularVelocity = nextAngularVelocity;
	    }
	}
    //copy parent property to its sons
    FOR_EACH_ENTITY_COMPONENT(Physics, a, pc)
		if (!TRANSFORM(a))
			continue;

		Entity parent = TRANSFORM(a)->parent;
		if (parent) {
			while (TRANSFORM(parent)->parent) {
				parent = TRANSFORM(parent)->parent;
			}

            PhysicsComponent* ppc = thePhysicsSystem.Get(parent, false);
            if (ppc) {
    			pc->linearVelocity = ppc->linearVelocity;
    			pc->angularVelocity = ppc->angularVelocity;
            } else {
                pc->linearVelocity = Vector2::Zero;
                pc->angularVelocity = 0;
            }
		}
    }
}


void PhysicsSystem::addMoment(PhysicsComponent* pc, float m) {
	// add 2 opposed forces
	pc->forces.push_back(std::make_pair(Force(Vector2(0, m * 0.5), Vector2(1, 0)), 0.016));
	pc->forces.push_back(std::make_pair(Force(Vector2(0, -m * 0.5), Vector2(-1, 0)), 0.016));
}

#ifdef SAC_INGAME_EDITORS
void PhysicsSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    PhysicsComponent* tc = Get(entity, false);
    if (!tc) return;
    TwAddVarRW(bar, "velocity.X", TW_TYPE_FLOAT, &tc->linearVelocity.X, "group=Physics precision=2 step=0,01");
    TwAddVarRW(bar, "velocity.Y", TW_TYPE_FLOAT, &tc->linearVelocity.Y, "group=Physics precision=2 step=0,01"); 
    TwAddVarRW(bar, "angularVelocity", TW_TYPE_FLOAT, &tc->angularVelocity, "group=Physics step=0,01 precision=2");
    TwAddVarRW(bar, "mass", TW_TYPE_FLOAT, &tc->mass, "group=Physics precision=1");
    TwAddVarRW(bar, "gravity.X", TW_TYPE_FLOAT, &tc->gravity.X, "group=Physics precision=2 step=0,01");
    TwAddVarRW(bar, "gravity.Y", TW_TYPE_FLOAT, &tc->gravity.Y, "group=Physics precision=2 step=0,01");
}
#endif