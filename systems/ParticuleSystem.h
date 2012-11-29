/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
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

