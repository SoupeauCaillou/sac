#pragma once

#include "../SoundAPI.h"
#if ! SAC_EMSCRIPTEN

#else
#include <SDL/SDL_mixer.h>
#endif
class AssetAPI;


class SoundAPILinuxOpenALImpl : public SoundAPI {
    public:
        ~SoundAPILinuxOpenALImpl();
        void init(AssetAPI* assetAPI, bool openALAlreadyInit);

        OpaqueSoundPtr* loadSound(const std::string& asset);
        bool play(OpaqueSoundPtr* p, float volume);

    private:
        AssetAPI* assetAPI;
#if ! SAC_EMSCRIPTEN
        unsigned int* soundSources;
#endif
};


