#pragma once

#include "System.h"

#include "base/Frequency.h"

struct AutoDestroyComponent {
    AutoDestroyComponent() : hasText(false) { }

    enum {
        OUT_OF_AREA,
        LIFETIME
    } type;

    std::function<void(Entity)> onDeletionCall;

    union _params {
        _params() {
            memset(this, 0, sizeof(_params));
        }

        struct _area : glm::vec2 {
            _area() : position(1.f), size(1.f) {}
            glm::vec2 position, size;
        } area;
        struct _lifetime {
            Frequency<float> freq;
            bool map2AlphaRendering, map2AlphaText;
        } lifetime;
    } params;
    // ARG TODO
    bool hasText;
};

#define theAutoDestroySystem AutoDestroySystem::GetInstance()
#define AUTO_DESTROY(e) theAutoDestroySystem.Get(e)

UPDATABLE_SYSTEM(AutoDestroy)

};
