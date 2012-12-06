#pragma once
#include <stdint.h>
#include <string>

struct OpaqueSoundPtr { };

class SoundAPI {
    public:
        virtual OpaqueSoundPtr* loadSound(const std::string& asset) = 0;
        virtual bool play(OpaqueSoundPtr* p, float volume) = 0;
};


