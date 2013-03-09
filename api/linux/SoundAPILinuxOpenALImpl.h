#pragma once

#include "../SoundAPI.h"
#ifndef EMSCRIPTEN
#include <al.h>
#include <alc.h>
#else
#include <SDL/SDL_mixer.h>
#endif
class SoundAPILinuxOpenALImpl : public SoundAPI {
    public:
        void init();
        OpaqueSoundPtr* loadSound(const std::string& asset);
        bool play(OpaqueSoundPtr* p, float volume);

    private:
    #ifndef EMSCRIPTEN
        ALuint soundSources[16];
    #endif
};


