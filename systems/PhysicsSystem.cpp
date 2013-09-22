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

#include "util/DrawSomething.h"

#include <utility>

INSTANCE_IMPL(PhysicsSystem);

PhysicsSystem::PhysicsSystem() : ComponentSystemImpl<PhysicsComponent>("Physics") {
    PhysicsComponent tc;
    componentSerializer.add(new Property<glm::vec2>("linear_velocity", OFFSET(linearVelocity, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<float>("angular_velocity", OFFSET(angularVelocity, tc), 0.001));
    componentSerializer.add(new Property<float>("mass", OFFSET(mass, tc), 0.001));
    componentSerializer.add(new Property<float>("frottement", OFFSET(frottement, tc), 0.001));
    componentSerializer.add(new Property<glm::vec2>("gravity", OFFSET(gravity, tc), glm::vec2(0.001, 0)));
}

#if SAC_DEBUG
void PhysicsSystem::addDebugOnlyDrawForce(const glm::vec2 & pos, const glm::vec2 & size) {
    return;
    float norm2 = glm::length2(size);
    if (norm2 < 0.00001f)
        return;

    norm2Max = glm::max(norm2Max, norm2);


    if (currentDraw == drawForceVectors.size()) {
        std::pair<Entity, std::vector<glm::vec2>> couple;

        couple.first = DrawSomething::DrawVec2("PhysicsDebug", pos, size, true);
        couple.second.push_back(pos);
        couple.second.push_back(size);
        drawForceVectors.push_back(couple);
    } else {
        drawForceVectors[currentDraw].second[0] = pos;
        drawForceVectors[currentDraw].second[1] = size;
    }

    ++currentDraw;
}
#else
#define addDebugOnlyDrawForce(a,b) {}
#endif

void PhysicsSystem::DoUpdate(float dt) {
#if SAC_DEBUG
    currentDraw = 0;
    norm2Max = 0.f;
#endif

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

		TransformationComponent* tc = TRANSFORM(a);

		pc->momentOfInertia = pc->mass * tc->size.x * tc->size.y / 6;

		// linear accel
		glm::vec2 linearAccel(pc->gravity * pc->mass);

        addDebugOnlyDrawForce(tc->position, pc->gravity * pc->mass);

		// angular accel
		float angAccel = 0;

        if (pc->frottement != 0.f) {
            pc->addForce(glm::vec2(0.f), - pc->frottement * pc->linearVelocity, dt);
        }

		for (unsigned int i=0; i<pc->forces.size(); i++) {
			Force force(pc->forces[i].first);

            addDebugOnlyDrawForce(tc->position + force.point, force.vector);

			float& durationLeft = pc->forces[i].second;

			if (durationLeft < dt) {
				force.vector *= durationLeft / dt;
			}

			linearAccel += force.vector;

	        if (force.point != glm::vec2(0.0f, 0.0f)) {
		        angAccel += glm::dot(glm::vec2(- force.point.y, force.point.x), force.vector);
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
        const glm::vec2 nextVelocity(pc->linearVelocity + linearAccel * dt);
        tc->position += (pc->linearVelocity + nextVelocity) * dt * 0.5f;
        // velocity varies over dt: use Verlet integration for position
        pc->linearVelocity = nextVelocity;
		const float nextAngularVelocity = pc->angularVelocity + angAccel * dt;
		tc->rotation += (pc->angularVelocity + nextAngularVelocity) * dt * 0.5;
        pc->angularVelocity = nextAngularVelocity;
	END_FOR_EACH()

#if SAC_DEBUG
    const float sizeForMaxForce = 2.f;
    float normMax = glm::sqrt(norm2Max) / sizeForMaxForce;

    for (unsigned i = 0; i < currentDraw; ++i) {
        glm::vec2 pos = drawForceVectors[i].second[0];
        glm::vec2 size = drawForceVectors[i].second[1];

        size /= normMax;
        float currentNorm = glm::length(size);

        //force vectors size must be in [0;sizeForMaxForce] (sizeForMaxForce = max vector)
        //but if the final force size is too small, change color/size
        if (currentNorm < 0.1f * sizeForMaxForce) {
            size = 0.1f * sizeForMaxForce * glm::normalize(size);
            //RENDERING(drawForceVectors[i].first)->color = Color(0.5,1.,0.,1.);
        } else {
            //RENDERING(drawForceVectors[i].first)->color = Color(1.,1.,1.,1.);
        }


        DrawSomething::DrawVec2("PhysicsDebug", pos, size, true, "force", drawForceVectors[i].first);
    }

    for (unsigned i = currentDraw; i < drawForceVectors.size(); ++i) {
        RENDERING(drawForceVectors[i].first)->show = false;
    }
#endif
}


void PhysicsSystem::addMoment(PhysicsComponent* pc, float m) {
	// add 2 opposed forces
    //WARNING: shouldn't be +size,0 and -size,0 instead of 1,0 / -1, 0?
	pc->addForce(glm::vec2(1, 0), glm::vec2(0, m * 0.5), 0.016);
	pc->addForce(glm::vec2(-1, 0), glm::vec2(0, -m * 0.5), 0.016);
}
