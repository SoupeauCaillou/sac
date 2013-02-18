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
void AutoDestroySystem::addEntityPropertiesToBar(Entity e, TwBar* bar) {

}
#endif
