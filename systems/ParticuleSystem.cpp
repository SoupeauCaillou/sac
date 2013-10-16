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



#include "ParticuleSystem.h"
#include "TransformationSystem.h"
#include "PhysicsSystem.h"
#include "base/EntityManager.h"

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/random.hpp>



#define MAX_PARTICULE_COUNT 4096

INSTANCE_IMPL(ParticuleSystem);

ParticuleSystem::ParticuleSystem() : ComponentSystemImpl<ParticuleComponent>("Particule") {
    /* nothing saved */
    minUsedIdx = maxUsedIdx = 0;

    ParticuleComponent tc;
    componentSerializer.add(new Property<float>("emission_rate", OFFSET(emissionRate, tc)));
    componentSerializer.add(new Property<float>("duration", OFFSET(duration, tc)));
    componentSerializer.add(new Property<TextureRef>("texture", PropertyType::Texture, OFFSET(texture, tc), 0));
    componentSerializer.add(new IntervalProperty<float>("lifetime", OFFSET(lifetime, tc)));
    componentSerializer.add(new IntervalProperty<Color>("initial_color", OFFSET(initialColor, tc)));
    componentSerializer.add(new IntervalProperty<Color>("final_color", OFFSET(finalColor, tc)));
    componentSerializer.add(new IntervalProperty<float>("initial_size", OFFSET(initialSize, tc)));
    componentSerializer.add(new IntervalProperty<float>("final_size", OFFSET(finalSize, tc)));
    componentSerializer.add(new IntervalProperty<float>("force_direction", OFFSET(forceDirection, tc)));
    componentSerializer.add(new IntervalProperty<float>("force_amplitude", OFFSET(forceAmplitude, tc)));
    componentSerializer.add(new IntervalProperty<float>("moment", OFFSET(moment, tc)));
    componentSerializer.add(new Property<float>("mass", OFFSET(mass, tc)));
    componentSerializer.add(new Property<glm::vec2>("gravity", OFFSET(gravity, tc), glm::vec2(0.001, 0)));
}

void ParticuleSystem::DoUpdate(float dt) {
 // return;
    FOR_EACH_ENTITY_COMPONENT(Particule, a, pc)
        TransformationComponent* ptc = TRANSFORM(a);
        if (pc->duration >= 0) {
        	pc->duration -= dt;
        	if (pc->duration <= 0) {
        		pc->duration = 0;
        		continue;
         	}
        }

        // emit particules
        if (pc->emissionRate > 0) {
#if SAC_DEBUG
            std::stringstream name;
            name << "particule_" << theEntityManager.entityName(a);
#endif
            int count = pc->emissionRate * (dt + pc->spawnLeftOver);
            pc->spawnLeftOver += dt - count / pc->emissionRate;
            count = glm::min(count, MAX_PARTICULE_COUNT - (int)particules.size());
            particules.resize(particules.size() + count);
            std::list<InternalParticule>::reverse_iterator intP = particules.rbegin();
            for (int i=0; i<count; i++) {
                InternalParticule& internal = *intP++;
                internal.time = -dt;
                internal.lifetime = pc->lifetime.random();

#if SAC_DEBUG
                Entity e = internal.e = theEntityManager.CreateEntity(name.str());
#else
                Entity e = internal.e = theEntityManager.CreateEntity();
#endif
                ADD_COMPONENT(e, Transformation);
                TransformationComponent* tc = TRANSFORM(e);
                tc->position = ptc->position + glm::rotate(glm::vec2(glm::linearRand(-0.5f, 0.5f) * ptc->size.x, glm::linearRand(-0.5f, 0.5f) * ptc->size.y), ptc->rotation);
                tc->rotation = ptc->rotation;
                tc->size.x = tc->size.y = pc->initialSize.random();
                tc->z = ptc->z;

                ADD_COMPONENT(e, Rendering);
                RenderingComponent* rc = RENDERING(e);
                rc->fastCulling = true;
                rc->color = pc->initialColor.random();
                rc->texture = pc->texture;
                rc->show = true;
                if (pc->texture == InvalidTextureRef && pc->initialColor.t1.a == 1 && pc->initialColor.t2.a == 1)
                    rc->opaqueType = RenderingComponent::FULL_OPAQUE;
                if (pc->mass) {
                    ADD_COMPONENT(e, Physics);
                    PhysicsComponent* ppc = PHYSICS(e);
                    ppc->gravity = pc->gravity;
                    ppc->mass = pc->mass;
                    float angle = ptc->rotation + pc->forceDirection.random();
                    ppc->addForce(
                        glm::vec2(
                            glm::cos(angle),
                            glm::sin(angle)
                        ) * pc->forceAmplitude.random(),
                        glm::vec2(0.0f),//tc->size * glm::linearRand(-0.5f, 0.5f),
                        0.016);
                    PhysicsSystem::addMoment(ppc, pc->moment.random());
                }
                internal.color = Interval<Color> (rc->color, pc->finalColor.random());
                internal.size = Interval<float> (tc->size.x, pc->finalSize.random());
            }
        }
    END_FOR_EACH()

    // update emitted particules
    for (std::list<InternalParticule>::iterator it=particules.begin(); it!=particules.end(); ) {
        InternalParticule& internal = *it;
        internal.time += dt;

        if (internal.time >= internal.lifetime) {
            std::list<InternalParticule>::iterator next = it;
            next++;
            theEntityManager.DeleteEntity(internal.e);
            internal.e = 0;
            particules.erase(it);
            it = next;
        } else {
            RENDERING(internal.e)->color = internal.color.lerp(internal.time / internal.lifetime);
            TRANSFORM(internal.e)->size = glm::vec2(internal.size.lerp(internal.time / internal.lifetime));
            ++it;
        }
    }
}
