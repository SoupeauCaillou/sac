#pragma once

#include "System.h"
#include "RenderingSystem.h"

class AnimDescriptor;

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

    void loadAnim(const std::string& name);

    AssetAPI* assetAPI;
private:
    std::map<std::string, AnimDescriptor*> animations;
    typedef std::map<std::string, AnimDescriptor*>::iterator AnimIt;
};
