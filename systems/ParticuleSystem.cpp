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

ParticuleSystem::ParticuleSystem() : ComponentSystemImpl<ParticuleComponent>(HASH("Particule", 0x52ec2829)) {
    /* nothing saved */
    minUsedIdx = maxUsedIdx = 0;

    ParticuleComponent tc;
    componentSerializer.add(new Property<float>(HASH("emission_rate", 0x9b57fb57), OFFSET(emissionRate, tc)));
    componentSerializer.add(new Property<float>(HASH("duration", 0x150075fa), OFFSET(duration, tc)));
    componentSerializer.add(new Property<TextureRef>(HASH("texture", 0x3d4e3ff8), PropertyType::Texture, OFFSET(texture, tc), 0));
    componentSerializer.add(new IntervalProperty<float>(HASH("lifetime", 0xae5c55fa), OFFSET(lifetime, tc)));
    componentSerializer.add(new IntervalProperty<Color>(HASH("initial_color", 0x973f18f9), OFFSET(initialColor, tc)));
    componentSerializer.add(new IntervalProperty<Color>(HASH("final_color", 0xe9f8bec0), OFFSET(finalColor, tc)));
    componentSerializer.add(new IntervalProperty<float>(HASH("initial_size", 0x56db655d), OFFSET(initialSize, tc)));
    componentSerializer.add(new IntervalProperty<float>(HASH("final_size", 0xa8e54f9), OFFSET(finalSize, tc)));
    componentSerializer.add(new IntervalProperty<float>(HASH("force_direction", 0x884663a6), OFFSET(forceDirection, tc)));
    componentSerializer.add(new IntervalProperty<float>(HASH("force_amplitude", 0xda0c587), OFFSET(forceAmplitude, tc)));
    componentSerializer.add(new IntervalProperty<float>(HASH("moment", 0xddd35795), OFFSET(moment, tc)));
    componentSerializer.add(new Property<float>(HASH("mass", 0xbfe03e46), OFFSET(mass, tc)));
    componentSerializer.add(new Property<glm::vec2>(HASH("gravity", 0x4db1fe87), OFFSET(gravity, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<int8_t>(HASH("rendering_flags", 0x77a0455a), OFFSET(renderingFlags, tc)));

    poolLastValidElement = -1;
}

void ParticuleSystem::DoUpdate(float dt) {
    std::vector<Entity> recyclable;

    // update emitted particules
    {
        unsigned eraseFromIndex = 0, eraseCount = 0;
        unsigned count = particules.size();
        for (unsigned i=0; i<count; i++) {
            InternalParticule& internal = particules[i];
            internal.time += dt;

            if (internal.time >= internal.lifetime) {
                recyclable.push_back(internal.e);
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
            } else {
                const float prog = internal.time / internal.lifetime;
                RENDERING(internal.e)->color = internal.color.lerp(prog);
                TRANSFORM(internal.e)->size = glm::vec2(internal.size.lerp(prog));
            }
        }

        if (eraseCount) {
            particules.erase(
                particules.begin() + eraseFromIndex,
                particules.begin() + eraseFromIndex + eraseCount
            );
        }
    }

    // then spawn particule
    // store in a float so a 0.83 value will go in the 'spawnLeftOver' var
    float spawnCount = 0.0f;
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
            spawnCount += pc->emissionRate * (dt + pc->spawnLeftOver);
        }
    }
    return;

    int firstParticuleIndex = (int) particules.size();
    int recyclableCount = (int)recyclable.size();
    if (spawnCount > 0) {

        particules.resize(particules.size() + spawnCount);

        // recycle particule entities
        for (int i=0; i<(int)spawnCount && i<recyclableCount; i++) {
            particules[firstParticuleIndex + i].e = recyclable[i];
        }
        // create missing particules
        for (int i=recyclableCount; i<(int)spawnCount; i++) {
            Entity e = theEntityManager.CreateEntity(HASH("__/particule", 0xe08bc21));
            ADD_COMPONENT(e, Transformation);
            ADD_COMPONENT(e, Rendering);
            ADD_COMPONENT(e, Physics);
            particules[firstParticuleIndex + i].e = e;
        }
    }
    // last but not least, delete unused recyclable particules
    auto& mgr = theEntityManager;
    for (int i=(int)spawnCount; i<recyclableCount; i++) {
        mgr.DeleteEntity(recyclable[i]);
    }

    if (spawnCount == 0.0f)
        return;

    FOR_EACH_ENTITY_COMPONENT(Particule, a, pc)
        if (pc->duration == 0)
            continue;
        int added = pc->emissionRate * (dt + pc->spawnLeftOver);
        pc->spawnLeftOver += dt - added / pc->emissionRate;


        const TransformationComponent* ptc = TRANSFORM(a);
        for (int i=0; i<added; i++) {
            InternalParticule& internal = particules[firstParticuleIndex++];
            Entity e = internal.e;
            internal.time = 0;
            internal.lifetime = pc->lifetime.random();

            TransformationComponent* tc = TRANSFORM(e);
            tc->position = ptc->position + glm::rotate(glm::vec2(glm::linearRand(-0.5f, 0.5f) * ptc->size.x, glm::linearRand(-0.5f, 0.5f) * ptc->size.y), ptc->rotation);
            tc->rotation = ptc->rotation;
            tc->size.x = tc->size.y = pc->initialSize.random();
            tc->z = ptc->z;

            RenderingComponent* rc = RENDERING(e);
            rc->flags = pc->renderingFlags | RenderingFlags::FastCulling;
            rc->color = pc->initialColor.random();
            rc->texture = pc->texture;
            rc->show = true;

            PhysicsComponent* ppc = PHYSICS(e);
            if (pc->mass) {
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
            } else {
                ppc->mass = 0;
            }
            internal.color = Interval<Color> (rc->color, pc->finalColor.random());
            internal.size = Interval<float> (tc->size.x, pc->finalSize.random());
        }
    }
}
