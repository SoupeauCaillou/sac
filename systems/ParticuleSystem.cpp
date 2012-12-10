#include "ParticuleSystem.h"
#include "TransformationSystem.h"
#include "PhysicsSystem.h"
#include "base/EntityManager.h"

#define MAX_PARTICULE_COUNT 4096

INSTANCE_IMPL(ParticuleSystem);

ParticuleSystem::ParticuleSystem() : ComponentSystemImpl<ParticuleComponent>("Particule") {
    /* nothing saved */
    minUsedIdx = maxUsedIdx = 0;

    ParticuleComponent tc;
    componentSerializer.add(new Property(OFFSET(emissionRate, tc), sizeof(float)));
    componentSerializer.add(new Property(OFFSET(duration, tc), sizeof(float)));
    componentSerializer.add(new Property(OFFSET(texture, tc), sizeof(TextureRef)));
    componentSerializer.add(new IntervalProperty<float>(OFFSET(lifetime, tc)));
    componentSerializer.add(new IntervalProperty<Color>(OFFSET(initialColor, tc)));
    componentSerializer.add(new IntervalProperty<Color>(OFFSET(finalColor, tc)));
    componentSerializer.add(new IntervalProperty<float>(OFFSET(initialSize, tc)));
    componentSerializer.add(new IntervalProperty<float>(OFFSET(finalSize, tc)));
    componentSerializer.add(new IntervalProperty<float>(OFFSET(forceDirection, tc)));
    componentSerializer.add(new IntervalProperty<float>(OFFSET(forceAmplitude, tc)));
    componentSerializer.add(new IntervalProperty<float>(OFFSET(moment, tc)));
    componentSerializer.add(new Property(OFFSET(mass, tc), sizeof(float)));
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
            int count = pc->emissionRate * (dt + pc->spawnLeftOver);
            pc->spawnLeftOver += dt - count / pc->emissionRate;
            count = MathUtil::Min(count, MAX_PARTICULE_COUNT - (int)particules.size());
            particules.resize(particules.size() + count);
            std::list<InternalParticule>::reverse_iterator intP = particules.rbegin();
            for (int i=0; i<count; i++) {
                InternalParticule& internal = *intP++;
                internal.time = 0;
                internal.lifetime = pc->lifetime.random();

                Entity e = internal.e = theEntityManager.CreateEntity();
                ADD_COMPONENT(e, Transformation);
                TransformationComponent* tc = TRANSFORM(e);
                tc->position = ptc->worldPosition + Vector2::Rotate(Vector2(MathUtil::RandomFloatInRange(-0.5, 0.5) * ptc->size.X, MathUtil::RandomFloatInRange(-0.5, 0.5) * ptc->size.Y), ptc->worldRotation);
                tc->size.X = tc->size.Y = pc->initialSize.random();
                tc->z = ptc->worldZ;

                ADD_COMPONENT(e, Rendering);
                RenderingComponent* rc = RENDERING(e);
                rc->color = pc->initialColor.random();
                rc->texture = pc->texture;
                rc->hide = false;
                if (pc->texture == InvalidTextureRef && pc->initialColor.t1.a == 1 && pc->initialColor.t2.a == 1)
                    rc->opaqueType = RenderingComponent::FULL_OPAQUE;
                if (pc->mass) {
                    ADD_COMPONENT(e, Physics);
                    PhysicsComponent* ppc = PHYSICS(e);
                    ppc->gravity = Vector2(0, -10);
                    ppc->mass = pc->mass;
                    float angle = pc->forceDirection.random();
                    ppc->forces.push_back(std::make_pair(Force(Vector2::VectorFromAngle(angle) * pc->forceAmplitude.random(), tc->size * MathUtil::RandomFloat()), 0.016));
                    PhysicsSystem::addMoment(ppc, pc->moment.random());
                }
                internal.color = Interval<Color> (rc->color, pc->finalColor.random());
                internal.size = Interval<float> (tc->size.X, pc->finalSize.random());
            }
        }
    }

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
            TRANSFORM(internal.e)->size = Vector2(internal.size.lerp(internal.time / internal.lifetime));
            ++it;
        }
    }
}
