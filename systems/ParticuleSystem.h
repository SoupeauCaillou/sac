#pragma once

#include "base/MathUtil.h"
#include "base/Log.h"
#include "base/Interval.h"

#include "RenderingSystem.h"
#include "System.h"

#include <vector>

struct ParticuleComponent {
    float emissionRate;
    TextureRef texture;
    Interval<float> lifetime;
    Interval<Color> initialColor;
    Interval<Color> finalColor;
    Interval<float> initialSize;
    Interval<float> finalSize;
    Interval<float> forceDirection;
    Interval<float> forceAmplitude;
    float spawnLeftOver;
    float mass;
};

struct InternalParticule {
    Entity e;
    TextureRef texture;
    float time, lifetime;
    Interval<Color> color;
    Interval<float> size;
};

#define theParticuleSystem ParticuleSystem::GetInstance()
#define PARTICULE(actor) theParticuleSystem.Get(actor)
UPDATABLE_SYSTEM(Particule)

private:
    InternalParticule* particules;
    int minUsedIdx, maxUsedIdx;
};

