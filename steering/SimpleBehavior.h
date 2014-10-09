#pragma once

#include "base/Interval.h"

namespace Steering
{
    struct SeekParams {
        SeekParams() : weight(0) {}
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
        WanderParams() : change(0) {}
        float distance, radius, jitter;
        glm::vec2 target;
        Interval<float> pauseDuration;
        float change;
    };
}
