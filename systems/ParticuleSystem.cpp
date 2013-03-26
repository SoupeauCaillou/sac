#include "ParticuleSystem.h"
#include "TransformationSystem.h"
#include "PhysicsSystem.h"
#include "base/EntityManager.h"

#include "base/MathUtil.h"


#define MAX_PARTICULE_COUNT 4096

INSTANCE_IMPL(ParticuleSystem);

ParticuleSystem::ParticuleSystem() : ComponentSystemImpl<ParticuleComponent>("Particule") {
    /* nothing saved */
    minUsedIdx = maxUsedIdx = 0;

    ParticuleComponent tc;
    componentSerializer.add(new Property<float>(OFFSET(emissionRate, tc)));
    componentSerializer.add(new Property<float>(OFFSET(duration, tc)));
    componentSerializer.add(new Property<TextureRef>(OFFSET(texture, tc)));
    componentSerializer.add(new IntervalProperty<float>(OFFSET(lifetime, tc)));
    componentSerializer.add(new IntervalProperty<Color>(OFFSET(initialColor, tc)));
    componentSerializer.add(new IntervalProperty<Color>(OFFSET(finalColor, tc)));
    componentSerializer.add(new IntervalProperty<float>(OFFSET(initialSize, tc)));
    componentSerializer.add(new IntervalProperty<float>(OFFSET(finalSize, tc)));
    componentSerializer.add(new IntervalProperty<float>(OFFSET(forceDirection, tc)));
    componentSerializer.add(new IntervalProperty<float>(OFFSET(forceAmplitude, tc)));
    componentSerializer.add(new IntervalProperty<float>(OFFSET(moment, tc)));
    componentSerializer.add(new Property<float>(OFFSET(mass, tc)));
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
            #ifdef SAC_SAC_DEBUG
            std::stringstream name;
            name << "particule_" << theEntityManager.entityName(a);
            #endif
            int count = pc->emissionRate * (dt + pc->spawnLeftOver);
            pc->spawnLeftOver += dt - count / pc->emissionRate;
            count = MathUtil::Min(count, MAX_PARTICULE_COUNT - (int)particules.size());
            particules.resize(particules.size() + count);
            std::list<InternalParticule>::reverse_iterator intP = particules.rbegin();
            for (int i=0; i<count; i++) {
                InternalParticule& internal = *intP++;
                internal.time = 0;
                internal.lifetime = pc->lifetime.random();

                #ifdef SAC_SAC_DEBUG
                Entity e = internal.e = theEntityManager.CreateEntity(name.str());
                #else
                Entity e = internal.e = theEntityManager.CreateEntity();
                #endif
                ADD_COMPONENT(e, Transformation);
                TransformationComponent* tc = TRANSFORM(e);
                tc->position = tc->worldPosition = ptc->worldPosition + Vector2::Rotate(Vector2(MathUtil::RandomFloatInRange(-0.5, 0.5) * ptc->size.X, MathUtil::RandomFloatInRange(-0.5, 0.5) * ptc->size.Y), ptc->worldRotation);
                tc->size.X = tc->size.Y = pc->initialSize.random();
                tc->z = tc->worldZ = ptc->worldZ;

                ADD_COMPONENT(e, Rendering);
                RenderingComponent* rc = RENDERING(e);
                rc->fastCulling = true;
                rc->color = pc->initialColor.random();
                rc->texture = pc->texture;
                rc->hide = false;
                if (pc->texture == InvalidTextureRef && pc->initialColor.t1.a == 1 && pc->initialColor.t2.a == 1)
                    rc->opaqueType = RenderingComponent::FULL_OPAQUE;
                if (pc->mass) {
                    ADD_COMPONENT(e, Physics);
                    PhysicsComponent* ppc = PHYSICS(e);
                    ppc->gravity = pc->gravity;
                    ppc->mass = pc->mass;
                    float angle = ptc->worldRotation + pc->forceDirection.random();
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

#ifdef SAC_INGAME_EDITORS
void ParticuleSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    ParticuleComponent* tc = Get(entity, false);
    if (!tc) return;
    TwAddVarRW(bar, "emissionRate", TW_TYPE_FLOAT, &tc->emissionRate, "group=Particule min=0");
    TwAddVarRW(bar, "min lifetime", TW_TYPE_FLOAT, &tc->lifetime.t1, "group=ParticuleLifetime step=0,01 min=0");
    TwAddVarRW(bar, "max lifetime", TW_TYPE_FLOAT, &tc->lifetime.t2, "group=ParticuleLifetime step=0,01 min=0");
    TwAddVarRW(bar, "init color A", TW_TYPE_COLOR4F, &tc->initialColor.t1, "group=ParticuleColor");
    TwAddVarRW(bar, "init color B", TW_TYPE_COLOR4F, &tc->initialColor.t2, "group=ParticuleColor");
    TwAddVarRW(bar, "final color A", TW_TYPE_COLOR4F, &tc->finalColor.t1, "group=ParticuleColor");
    TwAddVarRW(bar, "final color B", TW_TYPE_COLOR4F, &tc->finalColor.t2, "group=ParticuleColor");
    TwAddVarRW(bar, "min initial size", TW_TYPE_FLOAT, &tc->initialSize.t1, "group=ParticuleSize step=0,01 min=0");
    TwAddVarRW(bar, "max initial size", TW_TYPE_FLOAT, &tc->initialSize.t2, "group=ParticuleSize step=0,01 min=0");
    TwAddVarRW(bar, "min final size", TW_TYPE_FLOAT, &tc->finalSize.t1, "group=ParticuleSize step=0,01 min=0");
    TwAddVarRW(bar, "max final size", TW_TYPE_FLOAT, &tc->finalSize.t2, "group=ParticuleSize step=0,01 min=0");
    TwAddVarRW(bar, "min force direction", TW_TYPE_FLOAT, &tc->forceDirection.t1, "group=ParticulePhysics step=0,01");
    TwAddVarRW(bar, "max force direction", TW_TYPE_FLOAT, &tc->forceDirection.t2, "group=ParticulePhysics step=0,01");
    TwAddVarRW(bar, "min force amplitude", TW_TYPE_FLOAT, &tc->forceAmplitude.t1, "group=ParticulePhysics step=0,1 min=0");
    TwAddVarRW(bar, "max force amplitude", TW_TYPE_FLOAT, &tc->forceAmplitude.t2, "group=ParticulePhysics step=0,1 min=0");
    TwAddVarRW(bar, "min moment", TW_TYPE_FLOAT, &tc->moment.t1, "group=ParticulePhysics step=0,01 min=0");
    TwAddVarRW(bar, "max moment", TW_TYPE_FLOAT, &tc->moment.t2, "group=ParticulePhysics step=0,01 min=0");
    TwAddVarRW(bar, "gravity.X", TW_TYPE_FLOAT, &tc->gravity.X, "group=ParticulePhysics step=0,01");
    TwAddVarRW(bar, "gravity.Y", TW_TYPE_FLOAT, &tc->gravity.Y, "group=ParticulePhysics step=0,01");
    TwAddVarRW(bar, "mass", TW_TYPE_FLOAT, &tc->mass, "group=ParticulePhysics step=0,01 min=0");

    std::stringstream groups;
    groups << TwGetBarName(bar) << '/' << "ParticuleLifetime group=Particule\t\n"
        << TwGetBarName(bar) << '/' << "ParticuleColor group=Particule\t\n"
        << TwGetBarName(bar) << '/' << "ParticuleSize group=Particule\t\n"
        << TwGetBarName(bar) << '/' << "ParticulePhysics group=Particule\t\n";
    TwDefine(groups.str().c_str());
}
#endif
