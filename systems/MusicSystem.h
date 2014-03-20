/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#pragma once

#include "System.h"
#include "../api/AssetAPI.h"
#include "../api/MusicAPI.h"
#include "util/OggDecoder.h"

typedef int MusicRef;

#define InvalidMusicRef -1

#if SAC_EMSCRIPTEN
    #include <SDL_mixer.h>
#endif

namespace MusicControl {
    enum Enum {
        Play,
        Stop,
        Pause
    };
}

struct MusicComponent {
    MusicComponent() : music(InvalidMusicRef), loopNext(InvalidMusicRef), previousEnding(InvalidMusicRef), master(0), positionI(0), volume(1), autoLoopName(""), control(MusicControl::Stop) {
        opaque[0] = opaque[1] = 0;
        fadeOut = fadeIn = 0;
        paused = false;
    }

    MusicRef music, loopNext;
    MusicRef previousEnding;
    MusicComponent* master;
    float loopAt; // sec
    int positionI; // in [0,1]
    float fadeOut, fadeIn; // sec
    float volume;
    float currentVolume; //handled by system - do not modify
    bool looped, paused;
    std::string autoLoopName;
    MusicControl::Enum control;

    // 2 opaque structure to allow overlapping looping
    struct OpaqueMusicPtr* opaque[2];
};

#define theMusicSystem MusicSystem::GetInstance()
#if SAC_DEBUG
#define MUSIC(e) theMusicSystem.Get(e,true,__FILE__,__LINE__)
#else
#define MUSIC(e) theMusicSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Music)

    public:
        ~MusicSystem();
        void init();
        bool isMuted() const { return muted; }

        MusicRef loadMusicFile(const std::string& assetName);

        void toggleMute(bool enable);

    private:
        /* textures cache */
        MusicRef nextValidRef;

        struct MusicInfo {
            MusicInfo() : handle(0) {}
            OggHandle* handle;
            // track info
            float totalTime;
            int nbSamples;
            int sampleRate, numChannels;

            float queuedDuration;
            bool toRemove;
        };
        std::map<MusicRef, MusicInfo> musics;

        bool muted;
        // map<filename, audio_compressed_content>
        std::map<std::string, FileBuffer> name2buffer;

        void feed(OpaqueMusicPtr* ptr, MusicRef m, float dt);

        OpaqueMusicPtr* startOpaque(MusicComponent* m, MusicRef r,
            MusicComponent* master, int offset);
        void stopMusic(MusicComponent* m);
        void clearAndRemoveInfo(MusicRef ref);
    public:
        MusicAPI* musicAPI;
        AssetAPI* assetAPI;
};

