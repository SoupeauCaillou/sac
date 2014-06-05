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



#include "PhysicsSystem.h"
#include "AnchorSystem.h"
#include "TransformationSystem.h"
#include "RenderingSystem.h"
#include <glm/gtx/perpendicular.hpp>
#include <glm/gtx/norm.hpp>

#include <utility>

INSTANCE_IMPL(PhysicsSystem);

PhysicsSystem::PhysicsSystem() : ComponentSystemImpl<PhysicsComponent>(HASH("Physics", 0xecfc0aba)) {
    PhysicsComponent tc;
    componentSerializer.add(new Property<glm::vec2>(HASH("linear_velocity", 0xba5da842), OFFSET(linearVelocity, tc), glm::vec2(0.001f, 0)));
    componentSerializer.add(new Property<float>(HASH("angular_velocity", 0x9d13e5d2), OFFSET(angularVelocity, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("mass", 0xbfe03e46), OFFSET(mass, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("frottement", 0xcbd83619), OFFSET(frottement, tc), 0.001f));
    componentSerializer.add(new Property<glm::vec2>(HASH("gravity", 0x4db1fe87), OFFSET(gravity, tc), glm::vec2(0.001f, 0)));
    componentSerializer.add(new Property<float>(HASH("max_speed", 0x3fbe6552), OFFSET(maxSpeed, tc), 0.001f));
}

void PhysicsSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Physics, a, pc)
        // no mass -> no physics
        if (pc->mass <= 0)
            continue;

#if SAC_DEBUG
        auto anchor = theAnchorSystem.Get(a, false);
        if (anchor && anchor->parent) {
            LOGW("Entity '"
                << theEntityManager.entityName(a)
                << "' tried to do physics while being anchored to '"
                << theEntityManager.entityName(anchor->parent)
                << "'");
            continue;
        }
#endif

        // linear accel
        glm::vec2 linearAccel(pc->gravity * pc->mass);

        // angular accel
        float angAccel = 0;

        if (pc->frottement != 0.f) {
            pc->addForce(- pc->frottement * pc->linearVelocity, glm::vec2(0.f), dt);
        }

        for (unsigned int i=0; i<pc->forces.size(); i++) {
            Force force(pc->forces[i].first);

            float& durationLeft = pc->forces[i].second;

            if (durationLeft < dt) {
                force.vector *= durationLeft / dt;
            }

            linearAccel += force.vector;
            if (force.point != glm::vec2(0.0f, 0.0f)) {
                angAccel += glm::dot(glm::vec2(- force.point.y, force.point.x), force.vector);
            }

            durationLeft -= dt;
            if (durationLeft <= 0.f) {
                pc->forces.erase(pc->forces.begin() + i);
                i--;
            }
        }

        linearAccel /= pc->mass;

        // acceleration is constant over dt: use basic Euler integration for velocity
        glm::vec2 nextVelocity(pc->linearVelocity + linearAccel * dt);
        // limit linearVelocity if requested
        if (pc->maxSpeed > 0) {
            float l2 = glm::length2(nextVelocity);
            if (l2 > (pc->maxSpeed * pc->maxSpeed)) {
                nextVelocity *= pc->maxSpeed / glm::sqrt(l2);
            }
        }

        TransformationComponent* tc = TRANSFORM(a);
        const float momentOfInertia = pc->mass * tc->size.x * tc->size.y / 6.0f;
        angAccel /= momentOfInertia;

        tc->position += (pc->linearVelocity + nextVelocity) * dt * 0.5f;
        // velocity varies over dt: use Verlet integration for position
        pc->linearVelocity = nextVelocity;
        const float nextAngularVelocity = pc->angularVelocity + angAccel * dt;
        tc->rotation += (pc->angularVelocity + nextAngularVelocity) * dt * 0.5f;
        pc->angularVelocity = nextAngularVelocity;

    END_FOR_EACH()
}


void PhysicsSystem::addMoment(PhysicsComponent* pc, float m) {
    // add 2 opposed forces
    //WARNING: shouldn't be +size,0 and -size,0 instead of 1,0 / -1, 0?
    pc->addForce(glm::vec2(0, m * 0.5), glm::vec2(1, 0), 0.016f);
    pc->addForce(glm::vec2(0, -m * 0.5), glm::vec2(-1, 0), 0.016f);
}
