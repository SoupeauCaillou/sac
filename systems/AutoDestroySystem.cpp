#include "AutoDestroySystem.h"

#include "base/EntityManager.h"

#include "systems/RenderingSystem.h"
#include "systems/TextRenderingSystem.h"

INSTANCE_IMPL(AutoDestroySystem);

AutoDestroySystem::AutoDestroySystem() : ComponentSystemImpl<AutoDestroyComponent>("AutoDestroy") {
    /* nothing saved */
}

void AutoDestroySystem::DoUpdate(float dt) {
    std::vector<std::pair<Entity, bool> > toRemove;
    FOR_EACH_ENTITY_COMPONENT(AutoDestroy, a, adc)
        switch (adc->type) {
            case AutoDestroyComponent::OUT_OF_SCREEN:
                if (!theRenderingSystem.isEntityVisible(a)) {
                    toRemove.push_back(std::make_pair(a, adc->hasTextRendering));
                }
                break;
            case AutoDestroyComponent::LIFETIME: {
                adc->params.lifetime.accum += dt;
                if (adc->params.lifetime.accum >= adc->params.lifetime.value) {
                    toRemove.push_back(std::make_pair(a, adc->hasTextRendering));
                } else {
                    if (adc->params.lifetime.map2AlphaRendering) {
                        RENDERING(a)->color.a = 1 - adc->params.lifetime.accum / adc->params.lifetime.value;
                    }
                    if (adc->params.lifetime.map2AlphaTextRendering) {
                        TEXT_RENDERING(a)->color.a = 1 - adc->params.lifetime.accum / adc->params.lifetime.value;
                    }
                }
                break;
            }
        }
    }
    for (unsigned i=0; i<toRemove.size(); i++) {
        const std::pair<Entity, bool>& p = toRemove[i];
        if (p.second) {
            theTextRenderingSystem.DeleteEntity(p.first);
        } else {
            theEntityManager.DeleteEntity(p.first);
        }
    }
}

#ifdef INGAME_EDITORS
void AutoDestroySystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    AutoDestroyComponent* tc = Get(entity, false);
    if (!tc) return;
    TwEnumVal modes[] = { {AutoDestroyComponent::OUT_OF_SCREEN, "Out of screen"}, {AutoDestroyComponent::LIFETIME, "Lifetime"} };
    TwType type = TwDefineEnum("Type", modes, 2);
    TwAddVarRW(bar, "type", type, &tc->type, "group=AutoDestroy");
    TwAddVarRW(bar, "ad_value", TW_TYPE_FLOAT, &tc->params.lifetime.value, "group=lifetime");
    TwAddVarRO(bar, "ad_accum", TW_TYPE_FLOAT, &tc->params.lifetime.accum, "group=lifetime");
    TwAddVarRW(bar, "map2AlphaRendering", TW_TYPE_BOOLCPP, &tc->params.lifetime.map2AlphaRendering, "group=lifetime");
    TwAddVarRW(bar, "map2AlphaTextRendering", TW_TYPE_BOOLCPP, &tc->params.lifetime.map2AlphaTextRendering, "group=lifetime");
    std::stringstream groups;
    groups << TwGetBarName(bar) << '/' << "lifetime group=AutoDestroy";
    TwDefine(groups.str().c_str());
}
#endif
