#pragma once

#include "base/Interval.h"
#include "TextureLibrary.h"
#include <vector>
#include <string>

class AnimDescriptor {
    public:
        std::vector<TextureRef> textures;
        float playbackSpeed;
        Interval<int> loopCount;
        std::string nextAnim;
        Interval<float> nextAnimWait;
};
