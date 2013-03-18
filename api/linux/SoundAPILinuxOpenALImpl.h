#pragma once

#include "../SoundAPI.h"
#ifndef EMSCRIPTEN

#else
#include <SDL/SDL_mixer.h>
#endif


class SoundAPILinuxOpenALImpl : public SoundAPI {
    public:
        ~SoundAPILinuxOpenALImpl();
        void init();

        OpaqueSoundPtr* loadSound(const std::string& asset);
        bool play(OpaqueSoundPtr* p, float volume);

    private:
    #ifndef EMSCRIPTEN
        unsigned int* soundSources;
    #endif
};


