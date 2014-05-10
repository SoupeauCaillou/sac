#include "Random.h"

#include "util/ReplayManager.h"
#include <random>

static std::mt19937 mt;

void Random::Init (unsigned int seed) {

    if (theReplayManager.isReplayModeEnabled()) {
        seed = theReplayManager.getRandomSeed();
    }
    //not a proper init but we don't mind (game!)
    if (seed == 0) {
    #if SAC_BENCHMARK_MODE
        mt.seed(0);
    #else
        mt.seed(time(0));
    #endif
    } else {
        mt.seed(seed);
    }

    srand(seed);

    if (!theReplayManager.isReplayModeEnabled()) {
        theReplayManager.saveRandomSeed(seed);
    }
}

float Random::Float (float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(mt);
}

void Random::N_Floats (int n, float* out, float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    for (int i=0; i<n; i++) {
        out[i] = dist(mt);
    }
}

int Random::Int (int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(mt);
}

void Random::N_Ints (int n, int* out, int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    for (int i=0; i<n; i++) {
        out[i] = dist(mt);
    }
}
