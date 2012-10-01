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
#pragma once

#include "System.h"
#include "RenderingSystem.h"

struct AutoDestroyComponent {
    AutoDestroyComponent() {
        params.lifetime.value = params.lifetime.accum = 0;
        params.lifetime.map2AlphaRendering = params.lifetime.map2AlphaTextRendering = false;
        hasTextRendering = false;
    }
    enum {
        OUT_OF_SCREEN,
        LIFETIME
    } type;

    union {
        struct {
            float value, accum;
            bool map2AlphaRendering, map2AlphaTextRendering;
        } lifetime;
    } params;
    // ARG TODO
    bool hasTextRendering;
};

#define theAutoDestroySystem AutoDestroySystem::GetInstance()
#define AUTO_DESTROY(e) theAutoDestroySystem.Get(e)

UPDATABLE_SYSTEM(AutoDestroy)

};
