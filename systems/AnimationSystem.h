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
#include "util/MurmurHash.h"

class AssetAPI;
class AnimDescriptor;

struct AnimationComponent {
    AnimationComponent()
        : name(0), previousName(0), accum(0), playbackSpeed(1), loopCount(-1),
          frameIndex(0) {
        waitAccum = 0;
    }
    hash_t name, previousName;
    float accum, playbackSpeed;
    int loopCount, frameIndex;
    float waitAccum;
};

#define theAnimationSystem AnimationSystem::GetInstance()
#if SAC_DEBUG
#define ANIMATION(e) theAnimationSystem.Get(e, true, __FILE__, __LINE__)
#else
#define ANIMATION(e) theAnimationSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Animation)

public:
~AnimationSystem();

void loadAnim(AssetAPI* assetAPI,
              const std::string& name,
              const std::string& file,
              std::string* variables = 0,
              int varcount = 0);

private:
std::map<uint32_t, AnimDescriptor*> animations;
}
;
