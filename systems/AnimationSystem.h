#pragma once

#include "System.h"
#include "RenderingSystem.h"

class AnimDescriptor;

struct AnimationComponent {
    AnimationComponent() : accum(0), playbackSpeed(1), loopCount(-1), frameIndex(0) {
        waitAccum = 0;
    }
    std::string name, previousName;
    float accum, playbackSpeed;
    int loopCount, frameIndex;
    float waitAccum;
    std::vector<Entity> subPart;
};

#define theAnimationSystem AnimationSystem::GetInstance()
#define ANIMATION(e) theAnimationSystem.Get(e)

UPDATABLE_SYSTEM(Animation)

public:
    ~AnimationSystem();

    void loadAnim(AssetAPI* assetAPI, const std::string& name, const std::string& file, std::string* variables = 0, int varcount = 0);

private:
    std::map<std::string, AnimDescriptor*> animations;
    typedef std::map<std::string, AnimDescriptor*>::iterator AnimIt;
};
