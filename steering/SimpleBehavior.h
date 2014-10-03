#pragma once

namespace Steering
{
    struct SeekParams {
        Entity target;
    };

    struct FleeParams {
        float radius;
        Entity target;
    };

    struct ArriveParams {
        Entity target;
        float breakingDistance;
    };

    struct WanderParams {
        float radius;
        float distance;
        float jitter;
        glm::vec2 target;
        #if SAC_DEBUG
        glm::vec2 debugTarget;
        #endif
    };
}
