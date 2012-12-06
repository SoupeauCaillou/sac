#pragma once

#include "base/MathUtil.h"
#include "base/Log.h"
#include "base/Interval.h"

#include "RenderingSystem.h"
#include "System.h"

#include <vector>
#include <list>

struct ParticuleComponent {
	ParticuleComponent() : emissionRate(0), duration(-1), texture(InvalidTextureRef), spawnLeftOver(0) {}
    float emissionRate, duration;
    TextureRef texture;
    Interval<float> lifetime;
    Interval<Color> initialColor;
    Interval<Color> finalColor;
    Interval<float> initialSize;
    Interval<float> finalSize;
    Interval<float> forceDirection;
    Interval<float> forceAmplitude;
    Interval<float> moment;
    float spawnLeftOver;
    float mass;
};

struct InternalParticule {
    Entity e;
    float time, lifetime;
    Interval<Color> color;
    Interval<float> size;
};

#define theParticuleSystem ParticuleSystem::GetInstance()
#define PARTICULE(actor) theParticuleSystem.Get(actor)
UPDATABLE_SYSTEM(Particule)

private:
    std::list<InternalParticule> particules;
    int minUsedIdx, maxUsedIdx;
};

