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

struct OggVorbis_File;
#include "System.h"
#include "../api/AssetAPI.h"
#include "../api/MusicAPI.h"
#include <thread>
#include <mutex>
#include <condition_variable>

typedef int MusicRef;

#define InvalidMusicRef -1
class CircularBuffer;

#if SAC_EMSCRIPTEN
    #include <SDL/SDL_mixer.h>
#endif

// #define SAC_MUSIC_VISU

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
    #if SAC_MUSIC_VISU
        float positionF;
    #endif
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
#define MUSIC(e) theMusicSystem.Get(e)

UPDATABLE_SYSTEM(Music)

    public:
        ~MusicSystem();
        void init();
        bool isMuted() const { return muted; }

        MusicRef loadMusicFile(const std::string& assetName);
        void unloadMusic(MusicRef ref);

        void toggleMute(bool enable);

    private:
        /* textures cache */
        MusicRef nextValidRef;

        #if SAC_EMSCRIPTEN
            std::map<MusicRef, Mix_Chunk*> musics;
        #else
            struct MusicInfo {
                MusicInfo() : ovf(0) {}
                OggVorbis_File* ovf;
                // track info
                float totalTime;
                int nbSamples, channels;
                int sampleRate;
                // raw data
                int pcmBufferSize;
                CircularBuffer* buffer;
                float leftOver;
                bool toRemove;
            #if SAC_MUSIC_VISU
                std::string name;
            #endif
            };
            std::map<MusicRef, MusicInfo> musics;
        #endif

        bool muted;
        #if ! SAC_EMSCRIPTEN
            std::thread oggDecompressionThread;
            std::mutex mutex;
            std::condition_variable cond;
            // map<filename, audio_compressed_content>
            std::map<std::string, FileBuffer> name2buffer;
        #endif

        #if SAC_MUSIC_VISU
            std::map<Entity, std::pair<Entity, Entity> > visualisationEntities;
        #endif

        #if ! SAC_EMSCRIPTEN
            int decompressNextChunk(OggVorbis_File* file, int8_t* data, int chunkSize);
            bool feed(OpaqueMusicPtr* ptr, MusicRef m, int forceCount, float dt);
        #endif
            
        OpaqueMusicPtr* startOpaque(MusicComponent* m, MusicRef r, 
            MusicComponent* master, int offset);
        void stopMusic(MusicComponent* m);
        void clearAndRemoveInfo(MusicRef ref);
    public:
        MusicAPI* musicAPI;
        AssetAPI* assetAPI;

        void oggDecompRunLoop();
        bool runDecompLoop;
};

