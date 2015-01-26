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

#pragma once

#include "base/Log.h"
#include "base/EntityManager.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"

#include "api/AdAPI.h"

class AdAPIDebugImpl : public AdAPI {
    public:
    AdAPIDebugImpl() : e(0) {}
    bool showAd(bool LOG_USAGE_ONLY(force)) {
        if (e == 0) {
            e = theEntityManager.CreateEntity(HASH("ad", 0x0));
            ADD_COMPONENT(e, Transformation);
            ADD_COMPONENT(e, Button);
            ADD_COMPONENT(e, Rendering);

            TRANSFORM(e)->z = 1.;
            TRANSFORM(e)->size = glm::vec2(100, 100);
        }
        BUTTON(e)->enabled = true;
        RENDERING(e)->show = true;
        RENDERING(e)->color = Color::random();

        LOGI("!!!!!!!!!!!!!!!!!!!!!!!!!!!!Interstitial ad display "
             << (force ? "(forced)" : "") << "!!!!!!!!!!!!!!!!!!!!!!!");
        return true;
    }

    bool done() {
        if (RENDERING(e)->show) {
            if (BUTTON(e)->clicked) {
                RENDERING(e)->show = false;
                BUTTON(e)->enabled = false;
                return true;
            }
            return false;
        }
        return true;
    }

    private:
    Entity e;
};
