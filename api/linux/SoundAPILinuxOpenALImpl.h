#pragma once

#include "../SoundAPI.h"
#include <AL/al.h>
#include <AL/alc.h>

class SoundAPILinuxOpenALImpl : public SoundAPI {
    public:
        void init();
        OpaqueSoundPtr* loadSound(const std::string& asset);
        void play(OpaqueSoundPtr* p, float volume);

    private:
        ALuint soundSources[16];
};


