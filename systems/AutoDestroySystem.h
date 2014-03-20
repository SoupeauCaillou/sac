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

#include "System.h"

#include "base/Frequency.h"

struct AutoDestroyComponent {
    AutoDestroyComponent() : hasText(false) { }

    enum {
        OUT_OF_AREA = 0,
        LIFETIME,
        NONE
    } type;

    std::function<void(Entity)> onDeletionCall;

    struct _params {
        _params() {
            memset(this, 0, sizeof(_params));
        }
        _params& operator=(const _params& p) {
            area = p.area;
            lifetime = p.lifetime;
            return *this;
        }

        struct _area : glm::vec2 {
            _area() : position(1.f), size(1.f) {}
            glm::vec2 position, size;
        } area;
        struct _lifetime {
            Frequency<float> freq;
            bool map2AlphaRendering, map2AlphaText;
        } lifetime;
    };
    _params params;
    // ARG TODO
    bool hasText;
};

#define theAutoDestroySystem AutoDestroySystem::GetInstance()
#if SAC_DEBUG
#define AUTO_DESTROY(e) theAutoDestroySystem.Get(e,true,__FILE__,__LINE__)
#else
#define AUTO_DESTROY(e) theAutoDestroySystem.Get(e)
#endif

UPDATABLE_SYSTEM(AutoDestroy)

};
