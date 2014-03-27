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
    componentSerializer.add(new Property<float>(HASH("emission_rate", 0x0), OFFSET(emissionRate, tc)));
    componentSerializer.add(new Property<float>(HASH("duration", 0x0), OFFSET(duration, tc)));
    componentSerializer.add(new Property<TextureRef>(HASH("texture", 0x0), PropertyType::Texture, OFFSET(texture, tc), 0));
    componentSerializer.add(new IntervalProperty<float>(HASH("lifetime", 0x0), OFFSET(lifetime, tc)));
    componentSerializer.add(new IntervalProperty<Color>(HASH("initial_color", 0x0), OFFSET(initialColor, tc)));
    componentSerializer.add(new IntervalProperty<Color>(HASH("final_color", 0x0), OFFSET(finalColor, tc)));
    componentSerializer.add(new IntervalProperty<float>(HASH("initial_size", 0x0), OFFSET(initialSize, tc)));
    componentSerializer.add(new IntervalProperty<float>(HASH("final_size", 0x0), OFFSET(finalSize, tc)));
    componentSerializer.add(new IntervalProperty<float>(HASH("force_direction", 0x0), OFFSET(forceDirection, tc)));
    componentSerializer.add(new IntervalProperty<float>(HASH("force_amplitude", 0x0), OFFSET(forceAmplitude, tc)));
    componentSerializer.add(new IntervalProperty<float>(HASH("moment", 0x0), OFFSET(moment, tc)));
    componentSerializer.add(new Property<float>(HASH("mass", 0x0), OFFSET(mass, tc)));
    componentSerializer.add(new Property<glm::vec2>(HASH("gravity", 0x0), OFFSET(gravity, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<int8_t>(HASH("rendering_flags", 0x0), OFFSET(renderingFlags, tc)));

    poolLastValidElement = -1;
}

#if SAC_USE_VECTOR_STORAGE
static bool InternalParticuleCompare(const InternalParticule& i1, const InternalParticule& i2) {
    return i1.e < i2.e;
}
#endif

void ParticuleSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Particule, a, pc)
        if (pc->duration >= 0) {
            pc->duration -= dt;
            if (pc->duration <= 0) {
                pc->duration = 0;
                continue;
            }
        }

        // emit particules
        if (pc->emissionRate > 0) {
            int added = pc->emissionRate * (dt + pc->spawnLeftOver);
            pc->spawnLeftOver += dt - added / pc->emissionRate;
            unsigned pCount = particules.size();
            added = glm::min(added, MAX_PARTICULE_COUNT - (int)pCount);
            particules.resize(pCount + added);

            {
                int missing = added - (poolLastValidElement + 1);
                pool.resize(glm::max(pool.size(), pool.size() + missing));
                for (int i=0; i<missing; i++) {
                    Entity e = pool[++poolLastValidElement] = theEntityManager.CreateEntity(HASH("_particule", 0x0));
                    ADD_COMPONENT(e, Transformation);
                    ADD_COMPONENT(e, Rendering);
                    ADD_COMPONENT(e, Physics);
                    theEntityManager.SuspendEntity(e);
                }
            }

            const TransformationComponent* ptc = TRANSFORM(a);
            for (int i=0; i<added; i++) {
                InternalParticule& internal = particules[pCount++];
                internal.time = -dt;
                internal.lifetime = pc->lifetime.random();

                Entity e = internal.e = pool[poolLastValidElement--];
                theEntityManager.ResumeEntity(e);

                TransformationComponent* tc = TRANSFORM(e);
                #if !SAC_USE_VECTOR_STORAGE
                internal.tc = tc;
                #endif
                tc->position = ptc->position + glm::rotate(glm::vec2(glm::linearRand(-0.5f, 0.5f) * ptc->size.x, glm::linearRand(-0.5f, 0.5f) * ptc->size.y), ptc->rotation);
                tc->rotation = ptc->rotation;
                tc->size.x = tc->size.y = pc->initialSize.random();
                tc->z = ptc->z;

                RenderingComponent* rc = RENDERING(e);
                #if !SAC_USE_VECTOR_STORAGE
                internal.rc = rc;
                #endif
                rc->flags = pc->renderingFlags | RenderingFlags::FastCulling;
                rc->color = pc->initialColor.random();
                rc->texture = pc->texture;
                rc->show = true;

                if (pc->mass) {
                    PhysicsComponent* ppc = PHYSICS(e);
                    ppc->linearVelocity = glm::vec2(0.0f);
                    ppc->angularVelocity = 0;
                    ppc->gravity = pc->gravity;
                    ppc->mass = pc->mass;
                    float angle = ptc->rotation + pc->forceDirection.random();
                    ppc->addForce(
                        glm::vec2(
                            glm::cos(angle),
                            glm::sin(angle)
                        ) * pc->forceAmplitude.random(),
                        glm::vec2(0.0f),//tc->size * glm::linearRand(-0.5f, 0.5f),
                        0.016f);
                    PhysicsSystem::addMoment(ppc, pc->moment.random());
                }
                internal.color = Interval<Color> (rc->color, pc->finalColor.random());
                internal.size = Interval<float> (tc->size.x, pc->finalSize.random());
            }
        }
    }

#if SAC_USE_VECTOR_STORAGE
    // particules.sort(InternalParticuleCompare);
#endif
    // update emitted particules
    unsigned eraseFromIndex = 0, eraseCount = 0;
    unsigned count = particules.size();
    for (unsigned i=0; i<count; i++) {
        InternalParticule& internal = particules[i];
        internal.time += dt;

        if (internal.time >= internal.lifetime) {
            poolLastValidElement++;
            // make sure pool is big enough
            if ((int)pool.size() < (poolLastValidElement + 1)) {
                pool.push_back(internal.e);
            } else {
                pool[poolLastValidElement] = internal.e;
            }
            theEntityManager.SuspendEntity(internal.e);
            internal.e = 0;

            if ((i - eraseFromIndex) == eraseCount) {
                eraseCount++;
            } else {
                if (eraseCount) {
                    particules.erase(
                        particules.begin() + eraseFromIndex,
                        particules.begin() + eraseFromIndex + eraseCount
                    );
                }

                i -= eraseCount;
                count -= eraseCount;

                eraseFromIndex = i;
                eraseCount = 1;
            }
            // i--;
        } else {
            #if SAC_USE_VECTOR_STORAGE
                RENDERING(internal.e)
            #else
                internal.rc
            #endif
                ->color = internal.color.lerp(internal.time / internal.lifetime);
            #if SAC_USE_VECTOR_STORAGE
                TRANSFORM(internal.e)
            #else
                internal.tc
            #endif
                ->size = glm::vec2(internal.size.lerp(internal.time / internal.lifetime));
        }
    }

    if (eraseCount) {
        particules.erase(
            particules.begin() + eraseFromIndex,
            particules.begin() + eraseFromIndex + eraseCount
        );
    }
}
