#pragma once

#include "base/Interval.h"
#include "TextureLibrary.h"
#include <vector>
#include <string>

struct FileBuffer;

class AnimDescriptor {
    public:
        bool load(const std::string& ctx, const FileBuffer& fb, std::string* variables = 0, int varcount = 0);

    public:
        struct AnimFrame {
            TextureRef texture;
            struct Transform {
                glm::vec2 position, size;
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
