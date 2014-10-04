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

    struct AvoidParams {
        Entity* entities;
        int count;
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
