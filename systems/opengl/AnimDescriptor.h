#pragma once

#include "base/Interval.h"
#include "TextureLibrary.h"
#include <vector>
#include <string>

class FileBuffer;

class AnimDescriptor {
    public:
        bool load(const FileBuffer& fb, std::string* variables = 0, int varcount = 0);

    public:
        struct AnimFrame {
            TextureRef texture;
            struct Transform {
                Vector2 position, size;
                float rotation;
            };
            std::vector<Transform> transforms;
        };
        
    public:
        std::vector<AnimFrame> frames;
        float playbackSpeed;
        Interval<int> loopCount;
        std::string nextAnim;
        Interval<float> nextAnimWait;
};
