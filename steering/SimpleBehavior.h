#pragma once

namespace Steering
{
    struct SeekParams {
        Entity* entities;
        float* weight;
        int count;
    };

    struct FleeParams {
        float radius;
        Entity target;
    };

    struct AvoidParams {
        Entity* entities;
        int count;
    };

    struct SeparationParams {
        Entity* entities;
        int count;
        float radius;
    };

    struct AlignmentParams {
        Entity* entities;
        int count;
        float radius;
    };

    struct CohesionParams {
        Entity* entities;
        int count;
        float radius;
    };

    struct GroupParams {
        Entity* entities;
        int count;
        float neighborRadius;
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
