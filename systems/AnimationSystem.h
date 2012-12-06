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

#include "System.h"
#include "RenderingSystem.h"
#include "../base/Interval.h"

struct AnimationComponent {
    AnimationComponent() : accum(0), playbackSpeed(1), loopCount(-1), textureIndex(0) {
        waitAccum = 0;
    }
    std::string name, previousName;
    float accum, playbackSpeed;
    int loopCount, textureIndex;
    float waitAccum;
};

#define theAnimationSystem AnimationSystem::GetInstance()
#define ANIMATION(e) theAnimationSystem.Get(e)

UPDATABLE_SYSTEM(Animation)

public:
    ~AnimationSystem();
    void registerAnim(const std::string& name, std::string* textureNames, int count, float playbackSpeed, Interval<int> loopCount, const std::string& nextanim="", Interval<float> nextAnimWait = Interval<float>(0,0));
    void registerAnim(const std::string& name, std::vector<TextureRef> textures, float playbackSpeed, Interval<int> loopCount, const std::string& nextanim="", Interval<float> nextAnimWait = Interval<float>(0,0));

private:
    struct Anim;

    std::map<std::string, Anim*> animations;
    typedef std::map<std::string, Anim*>::iterator AnimIt;
};
