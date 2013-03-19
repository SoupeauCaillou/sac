#pragma once

#include "System.h"

struct AutoDestroyComponent {
    AutoDestroyComponent() {
        memset(&params, 0, sizeof(params));
        hasTextRendering = false;
    }
    enum {
        OUT_OF_AREA,
        LIFETIME
    } type;

    union {
        struct {
            float x,y,w,h;
        } area;
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
