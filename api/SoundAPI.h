#pragma once
#include <stdint.h>
#include <string>

struct OpaqueSoundPtr { };

class SoundAPI {
    public:
        virtual void init() = 0;
        virtual OpaqueSoundPtr* loadSound(const std::string& asset) = 0;
        virtual void play(OpaqueSoundPtr* p, float volume) = 0;
};


