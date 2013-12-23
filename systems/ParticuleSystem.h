/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



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
    RenderingComponent::Opacity opaqueType;
};

struct TransformationComponent;

struct InternalParticule {
    Entity e;
    float time, lifetime;
    Interval<Color> color;
    Interval<float> size;

    RenderingComponent* rc;
    TransformationComponent* tc;
};

#define theParticuleSystem ParticuleSystem::GetInstance()
#if SAC_DEBUG
#define PARTICULE(actor) theParticuleSystem.Get(actor,true,__FILE__,__LINE__)
#else
#define PARTICULE(actor) theParticuleSystem.Get(actor)
#endif
UPDATABLE_SYSTEM(Particule)

private:
    std::list<InternalParticule> particules;
    int minUsedIdx, maxUsedIdx;
    std::vector<Entity> pool;
    int poolLastValidElement;
};
