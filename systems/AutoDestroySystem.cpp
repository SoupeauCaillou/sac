/*
 This file is part of Heriswap.

 @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
 @author Soupe au Caillou - Gautier Pelloux-Prayer

 Heriswap is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, version 3.

 Heriswap is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "AutoDestroySystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextRenderingSystem.h"
#include "base/EntityManager.h"

INSTANCE_IMPL(AutoDestroySystem);
 
AutoDestroySystem::AutoDestroySystem() : ComponentSystemImpl<AutoDestroyComponent>("AutoDestroy") { 
    /* nothing saved */
}

void AutoDestroySystem::DoUpdate(float dt) {
    for(ComponentIt it=components.begin(); it!=components.end();) {
        Entity a = (*it).first;         
        AutoDestroyComponent* adc = (*it).second;
        ++it;

        switch (adc->type) {
            case AutoDestroyComponent::OUT_OF_SCREEN:
                if (!theRenderingSystem.isEntityVisible(a)) {
                    if (adc->hasTextRendering) {
                        theTextRenderingSystem.DeleteEntity(a);
                    } else {
                        theEntityManager.DeleteEntity(a);
                    }
                }
                break;
            case AutoDestroyComponent::LIFETIME: {
                adc->params.lifetime.accum += dt;
                if (adc->params.lifetime.accum >= adc->params.lifetime.value) {
                    if (adc->hasTextRendering) {
                        theTextRenderingSystem.DeleteEntity(a);
                    } else {
                        theEntityManager.DeleteEntity(a);
                    }
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
}

