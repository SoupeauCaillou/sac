#pragma once


#include "System.h"

#include "RenderingSystem.h"

#include "base/Interval.h"

#include <list>


struct ParticuleComponent {
	ParticuleComponent() : emissionRate(10), duration(10), texture(InvalidTextureRef), spawnLeftOver(0) {
        lifetime.t1 = lifetime.t2 = 1;
        initialSize.t1 = initialSize.t2 = 1;
        finalSize.t1 = finalSize.t2 =
        forceDirection.t1 = forceDirection.t2 =
        forceAmplitude.t1 = forceAmplitude.t2 =
        moment.t1 = moment.t2 = spawnLeftOver =
        mass = 0.0f;
        gravity = glm::vec2(0, 0);
    }
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
    glm::vec2 gravity;
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
