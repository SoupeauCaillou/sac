#include "AutoDestroySystem.h"

#include "base/EntityManager.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextRenderingSystem.h"
#include "util/IntersectionUtil.h"

INSTANCE_IMPL(AutoDestroySystem);

AutoDestroySystem::AutoDestroySystem() : ComponentSystemImpl<AutoDestroyComponent>("AutoDestroy") {
    AutoDestroyComponent ac;
    componentSerializer.add(new Property<int>("type", OFFSET(type, ac), 0));
    componentSerializer.add(new Property<glm::vec2>("params.area.position", OFFSET(params.area.position, ac), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<glm::vec2>("params.area.size", OFFSET(params.area.size, ac), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<float>("params.lifetime.value", OFFSET(params.lifetime.freq.value, ac), 0.001f));
    componentSerializer.add(new Property<float>("params.lifetime.accum", OFFSET(params.lifetime.freq.accum, ac), 0.001f));
    componentSerializer.add(new Property<bool>("params.lifetime.map2AlphaRendering", OFFSET(params.lifetime.map2AlphaRendering, ac), false));
    componentSerializer.add(new Property<bool>("params.lifetime.map2AlphaTextRendering", OFFSET(params.lifetime.map2AlphaTextRendering, ac), false));
}

void AutoDestroySystem::DoUpdate(float dt) {
    std::vector<std::pair<Entity, bool> > toRemove;
    FOR_EACH_ENTITY_COMPONENT(AutoDestroy, a, adc)
        switch (adc->type) {
            case AutoDestroyComponent::OUT_OF_AREA: {
                LOGW_IF(adc->params.area.size.x <= 0 || adc->params.area.size.y <= 0, "Invalid area size: " << adc->params.area.size.x << "x" << adc->params.area.size.y)
                const TransformationComponent* tc = TRANSFORM(a);
                if (!IntersectionUtil::rectangleRectangle(tc->worldPosition, tc->size, tc->worldRotation,
                    adc->params.area.position, adc->params.area.size, 0)) {
                    toRemove.push_back(std::make_pair(a, adc->hasTextRendering));

                    LOGI("Entity " << theEntityManager.entityName(a) << " is out of area -> destroyed")
                }
                break;
            }
            case AutoDestroyComponent::LIFETIME: {
                adc->params.lifetime.freq.accum += dt;
                if (adc->params.lifetime.freq.accum >= adc->params.lifetime.freq.value) {
                    toRemove.push_back(std::make_pair(a, adc->hasTextRendering));
                    LOGI("Entity " << theEntityManager.entityName(a) << " lifetime is over -> destroyed")
                } else {
                    if (adc->params.lifetime.map2AlphaRendering) {
                        RENDERING(a)->color.a = 1 - adc->params.lifetime.freq.accum / adc->params.lifetime.freq.value;
                    }
                    if (adc->params.lifetime.map2AlphaTextRendering) {
                        TEXT_RENDERING(a)->color.a = 1 - adc->params.lifetime.freq.accum / adc->params.lifetime.freq.value;
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

#if SAC_INGAME_EDITORS
void AutoDestroySystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    AutoDestroyComponent* tc = Get(entity, false);
    if (!tc) return;
    TwEnumVal modes[] = { {AutoDestroyComponent::OUT_OF_AREA, "Out of area"}, {AutoDestroyComponent::LIFETIME, "Lifetime"} };
    TwType type = TwDefineEnum("Type", modes, 2);
    TwAddVarRW(bar, "type", type, &tc->type, "group=AutoDestroy");
    TwAddVarRW(bar, "area position X", TW_TYPE_FLOAT, &tc->params.area.position.x, "group=area");
    TwAddVarRW(bar, "area position Y", TW_TYPE_FLOAT, &tc->params.area.position.y, "group=area");
    TwAddVarRW(bar, "area width", TW_TYPE_FLOAT, &tc->params.area.size.x, "group=area min=0");
    TwAddVarRW(bar, "area height", TW_TYPE_FLOAT, &tc->params.area.size.y, "group=area min=0");
    TwAddVarRW(bar, "ad_value", TW_TYPE_FLOAT, &tc->params.lifetime.freq.value, "group=lifetime min=0");
    TwAddVarRO(bar, "ad_accum", TW_TYPE_FLOAT, &tc->params.lifetime.freq.accum, "group=lifetime");
    TwAddVarRW(bar, "map2AlphaRendering", TW_TYPE_BOOLCPP, &tc->params.lifetime.map2AlphaRendering, "group=lifetime");
    TwAddVarRW(bar, "map2AlphaTextRendering", TW_TYPE_BOOLCPP, &tc->params.lifetime.map2AlphaTextRendering, "group=lifetime");
    std::stringstream groups;
    groups << TwGetBarName(bar) << '/' << "lifetime group=AutoDestroy\t\n" << TwGetBarName(bar) << '/' << "area group=AutoDestroy\t\n";
    TwDefine(groups.str().c_str());
}
#endif
