#include "ParticuleSystem.h"
#include "TransformationSystem.h"
#include "PhysicsSystem.h"
#include "base/EntityManager.h"

#define MAX_PARTICULE_COUNT 4096

INSTANCE_IMPL(ParticuleSystem);
 
ParticuleSystem::ParticuleSystem() : ComponentSystemImpl<ParticuleComponent>("Particule") {
    particules = new InternalParticule[MAX_PARTICULE_COUNT];
    minUsedIdx = maxUsedIdx = 0;
    memset(particules, 0, MAX_PARTICULE_COUNT * sizeof(InternalParticule));
}

void ParticuleSystem::DoUpdate(float dt) {
    for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
        Entity a = (*it).first;
        ParticuleComponent* pc = (*it).second;
        TransformationComponent* ptc = TRANSFORM(a);

        // emit particules
        if (pc->emissionRate > 0) {
            int count = pc->emissionRate * (dt + pc->spawnLeftOver);
            pc->spawnLeftOver += dt - count / pc->emissionRate;
            for (int i=0; i<count; i++) {
                InternalParticule internal;
                internal.time = 0;
                internal.lifetime = pc->lifetime.random();

                Entity e = internal.e = theEntityManager.CreateEntity();
                ADD_COMPONENT(e, Transformation);
                TransformationComponent* tc = TRANSFORM(e);
                tc->position = ptc->worldPosition + Vector2(MathUtil::RandomFloatInRange(-0.5, 0.5) * ptc->size.X, MathUtil::RandomFloatInRange(-0.5, 0.5) * ptc->size.Y);
                tc->size = pc->initialSize.random();
                tc->z = ptc->z;

                ADD_COMPONENT(e, Rendering);
                RenderingComponent* rc = RENDERING(e);
                rc->color = pc->initialColor.random();
                rc->texture = internal.texture = pc->texture;
                rc->hide = false;

                if (pc->mass) {
                    ADD_COMPONENT(e, Physics);
                    PhysicsComponent* ppc = PHYSICS(e);
                    ppc->gravity = Vector2(0, -10);
                    ppc->mass = pc->mass;
                    float angle = pc->forceDirection.random();
                    ppc->forces.push_back(Force(Vector2::VectorFromAngle(angle) * pc->forceAmplitude.random(), tc->size * MathUtil::RandomFloat()));
                }
                internal.color = Interval<Color> (rc->color, pc->finalColor.random());
                internal.size = Interval<float> (tc->size.X, pc->finalSize.random());

                int j;
                for (j=0; j<MAX_PARTICULE_COUNT; j++) {
                    int idx = (j + maxUsedIdx) % MAX_PARTICULE_COUNT;
                    if(particules[idx].e == 0) {
                        particules[idx] = internal;
                        maxUsedIdx = MathUtil::Max(maxUsedIdx, idx);
                        minUsedIdx = MathUtil::Min(minUsedIdx, idx);
                        break;
                    }
                }
                if (i == MAX_PARTICULE_COUNT) {
                    // no place found...
                    LOGW("No place found for particule :'(");
                    theEntityManager.DeleteEntity(e);
                }
            }
        }
        // update emitted particules
        for (int i=minUsedIdx; i<maxUsedIdx; i++) {
            InternalParticule& internal = particules[i];
            if (internal.e == 0)
                continue;
            internal.time += dt;
            if (internal.time >= internal.lifetime) {
                theEntityManager.DeleteEntity(internal.e);
                internal.e = 0;
                if (i == minUsedIdx)
                    minUsedIdx = i+1;
            } else {
                RENDERING(internal.e)->color = internal.color.lerp(internal.time / internal.lifetime);
                TRANSFORM(internal.e)->size = internal.size.lerp(internal.time / internal.lifetime);
            }
        }
    }
}