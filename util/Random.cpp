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
        mt.seed(time(0));
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
    return Float(mt, min, max);
}

float Random::Float (std::mt19937& gen, float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(gen);
}

void Random::N_Floats (int n, float* out, float min, float max) {
    N_Floats(mt, n, out, min, max);
}

void Random::N_Floats (std::mt19937& gen, int n, float* out, float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    for (int i=0; i<n; i++) {
        out[i] = dist(gen);
    }
}

int Random::Int (int min, int max) {
    return Int(mt, min, max);
}

int Random::Int (std::mt19937& gen, int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen);
}


void Random::N_Ints (int n, int* out, int min, int max) {
    N_Ints(mt, n, out, min, max);
}

void Random::N_Ints (std::mt19937& gen, int n, int* out, int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    for (int i=0; i<n; i++) {
        out[i] = dist(gen);
    }
}
