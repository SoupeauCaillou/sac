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
