#pragma once

#include "System.h"

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
