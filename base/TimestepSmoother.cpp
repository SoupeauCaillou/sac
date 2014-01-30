#include "TimestepSmoother.h"

#include <vector>
#include <algorithm>

#define SMOOTH_WEIGHT 0.2

TimestepSmoother::TimestepSmoother() {
    // init with dt = 1.0/60 sec
    for (int i=0; i<11; i++) {
        timestepHistory[i] = 1.0f / 60.0f;
    }
    next = 0;
}

float TimestepSmoother::smooth(float dt) {
    // append new timestep
    timestepHistory[next] = dt;
    next = (next + 1) % HISTORY_SIZE;

    std::vector<float> vec (timestepHistory, timestepHistory + HISTORY_SIZE);
    std::sort(vec.begin(), vec.end());

    float mean = 0;
    for (int i=2; i<HISTORY_SIZE - 2; i++) {
        mean += vec[i];
    }
    mean /= (HISTORY_SIZE - 4);

    return mean * SMOOTH_WEIGHT + (1.0f - SMOOTH_WEIGHT) * dt;
}
