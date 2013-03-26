#include "AutoDestroySystem.h"

#include "base/EntityManager.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextRenderingSystem.h"
#include "util/IntersectionUtil.h"

INSTANCE_IMPL(AutoDestroySystem);

AutoDestroySystem::AutoDestroySystem() : ComponentSystemImpl<AutoDestroyComponent>("AutoDestroy") {
    /* nothing saved */
}

void AutoDestroySystem::DoUpdate(float dt) {
    std::vector<std::pair<Entity, bool> > toRemove;
    FOR_EACH_ENTITY_COMPONENT(AutoDestroy, a, adc)
        switch (adc->type) {
            case AutoDestroyComponent::OUT_OF_AREA: {
                LOGW_IF(adc->params.area.w <= 0 || adc->params.area.h <= 0, "Invalid area size: " << adc->params.area.w << "x" << adc->params.area.h)
                const TransformationComponent* tc = TRANSFORM(a);
                if (!IntersectionUtil::rectangleRectangle(tc->worldPosition, tc->size, tc->worldRotation,
                    Vector2(adc->params.area.x, adc->params.area.y), Vector2(adc->params.area.w, adc->params.area.h), 0)) {
                    toRemove.push_back(std::make_pair(a, adc->hasTextRendering));
                    LOGV(1, "Entity " << theEntityManager.entityName(a) << " is out of area -> destroyed")
                }
                break;
            }
            case AutoDestroyComponent::LIFETIME: {
                adc->params.lifetime.accum += dt;
                if (adc->params.lifetime.accum >= adc->params.lifetime.value) {
                    toRemove.push_back(std::make_pair(a, adc->hasTextRendering));
                    LOGV(1, "Entity " << theEntityManager.entityName(a) << " lifetime is over -> destroyed")
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
            theEntityManager.DeleteEntity(p.first);
        } else {
            theEntityManager.DeleteEntity(p.first);
        }
    }
}

#ifdef SAC_INGAME_EDITORS
void AutoDestroySystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    AutoDestroyComponent* tc = Get(entity, false);
    if (!tc) return;
    TwEnumVal modes[] = { {AutoDestroyComponent::OUT_OF_AREA, "Out of area"}, {AutoDestroyComponent::LIFETIME, "Lifetime"} };
    TwType type = TwDefineEnum("Type", modes, 2);
    TwAddVarRW(bar, "type", type, &tc->type, "group=AutoDestroy");
    TwAddVarRW(bar, "area position X", TW_TYPE_FLOAT, &tc->params.area.x, "group=area");
    TwAddVarRW(bar, "area position Y", TW_TYPE_FLOAT, &tc->params.area.y, "group=area");
    TwAddVarRW(bar, "area width", TW_TYPE_FLOAT, &tc->params.area.w, "group=area min=0");
    TwAddVarRW(bar, "area height", TW_TYPE_FLOAT, &tc->params.area.h, "group=area min=0");
    TwAddVarRW(bar, "ad_value", TW_TYPE_FLOAT, &tc->params.lifetime.value, "group=lifetime min=0");
    TwAddVarRO(bar, "ad_accum", TW_TYPE_FLOAT, &tc->params.lifetime.accum, "group=lifetime");
    TwAddVarRW(bar, "map2AlphaRendering", TW_TYPE_BOOLCPP, &tc->params.lifetime.map2AlphaRendering, "group=lifetime");
    TwAddVarRW(bar, "map2AlphaTextRendering", TW_TYPE_BOOLCPP, &tc->params.lifetime.map2AlphaTextRendering, "group=lifetime");
    std::stringstream groups;
    groups << TwGetBarName(bar) << '/' << "lifetime group=AutoDestroy\t\n" << TwGetBarName(bar) << '/' << "area group=AutoDestroy\t\n";
    TwDefine(groups.str().c_str());
}
#endif
