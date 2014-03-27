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



#include "AutoDestroySystem.h"

#include "base/EntityManager.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextSystem.h"
#include "util/IntersectionUtil.h"

INSTANCE_IMPL(AutoDestroySystem);

AutoDestroySystem::AutoDestroySystem() : ComponentSystemImpl<AutoDestroyComponent>("AutoDestroy") {
    AutoDestroyComponent ac;
    componentSerializer.add(new Property<int>(HASH("type", 0x0), OFFSET(type, ac), 0));
    componentSerializer.add(new Property<glm::vec2>(HASH("area/position", 0x0), OFFSET(params.area.position, ac), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<glm::vec2>(HASH("area/size", 0x0), OFFSET(params.area.size, ac), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<float>(HASH("lifetime/value", 0x0), OFFSET(params.lifetime.freq.value, ac), 0.001f));
    componentSerializer.add(new Property<bool>(HASH("lifetime/map2AlphaRendering", 0x0), OFFSET(params.lifetime.map2AlphaRendering, ac), false));
    componentSerializer.add(new Property<bool>(HASH("lifetime/map2AlphaText", 0x0), OFFSET(params.lifetime.map2AlphaText, ac), false));
}

void AutoDestroySystem::DoUpdate(float dt) {
    std::vector<std::pair<Entity, bool> > toRemove;
    FOR_EACH_ENTITY_COMPONENT(AutoDestroy, a, adc)
        switch (adc->type) {
            case AutoDestroyComponent::OUT_OF_AREA: {
                LOGW_IF(adc->params.area.size.x <= 0 || adc->params.area.size.y <= 0, "Invalid area size: " << adc->params.area.size.x << "x" << adc->params.area.size.y);
                const TransformationComponent* tc = TRANSFORM(a);
                if (!IntersectionUtil::rectangleRectangle(tc->position, tc->size, tc->rotation,
                    adc->params.area.position, adc->params.area.size, 0)) {
                    toRemove.push_back(std::make_pair(a, adc->hasText));

                    LOGV(1, "Entity " << theEntityManager.entityName(a) << " is out of area -> destroyed ("
                        << tc->position << " not in " << adc->params.area.position << " x " << adc->params.area.position + adc->params.area.size);
                }
                break;
            }
            case AutoDestroyComponent::LIFETIME: {
                adc->params.lifetime.freq.accum += dt;
                if (adc->params.lifetime.freq.accum >= adc->params.lifetime.freq.value) {
                    toRemove.push_back(std::make_pair(a, adc->hasText));
                    LOGV(1, "Entity " << theEntityManager.entityName(a) << " lifetime is over -> destroyed");
                } else {
                    if (adc->params.lifetime.map2AlphaRendering) {
                        RENDERING(a)->color.a = 1 - adc->params.lifetime.freq.accum / adc->params.lifetime.freq.value;
                    }
                    if (adc->params.lifetime.map2AlphaText) {
                        TEXT(a)->color.a = 1 - adc->params.lifetime.freq.accum / adc->params.lifetime.freq.value;
                    }
                }
                break;
            }
            default: {
                break;
            }
        }
    END_FOR_EACH()
    for (unsigned i=0; i<toRemove.size(); i++) {
        const std::pair<Entity, bool>& p = toRemove[i];

        if (AUTO_DESTROY(p.first)->onDeletionCall) {
            LOGI("Calling onDeletionCall function for " << theEntityManager.entityName(p.first));
            AUTO_DESTROY(p.first)->onDeletionCall(p.first);
        }

        if (p.second) {
            theEntityManager.DeleteEntity(p.first);
        } else {
            theEntityManager.DeleteEntity(p.first);
        }
    }
}
