/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <map>

#include "base/MathUtil.h"
#include "base/Log.h"
#ifdef ANDROID
#include "tremor/ivorbisfile.h"
#else
#include <vorbis/vorbisfile.h>
#endif
#include "System.h"
#include "../api/AssetAPI.h"
#include "../api/MusicAPI.h"
#include <pthread.h>

typedef int MusicRef;

#define InvalidMusicRef -1
class CircularBuffer;

// #define MUSIC_VISU

struct MusicComponent {
	MusicComponent() : music(InvalidMusicRef), loopNext(InvalidMusicRef), previousEnding(InvalidMusicRef), master(0), positionI(0), volume(1), control(Stop) {
		opaque[0] = opaque[1] = 0;
		fadeOut = fadeIn = 0;
	}

    MusicRef music, loopNext;
    MusicRef previousEnding;
    MusicComponent* master;
    float loopAt; // sec
    int positionI; // in [0,1]
    #ifdef MUSIC_VISU
    float positionF;
    #endif
    float fadeOut, fadeIn; // sec
    float volume, currentVolume;
    bool looped;
    enum {
        Start, Stop
    } control;

    // 2 opaque structure to allow overlapping looping
    struct OpaqueMusicPtr* opaque[2];
};

#define theMusicSystem MusicSystem::GetInstance()
#define MUSIC(e) theMusicSystem.Get(e)

UPDATABLE_SYSTEM(Music)

public:
void init();
void uninit();
bool isMuted() const { return muted; }

MusicRef loadMusicFile(const std::string& assetName);
void unloadMusic(MusicRef ref);

void toggleMute(bool enable);

private:
/* textures cache */
MusicRef nextValidRef;

struct MusicInfo {
    MusicInfo() : ovf(0) {}
    OggVorbis_File* ovf;
    // track info
    float totalTime;
    int nbSamples;
    int sampleRate;
    // raw data
    int pcmBufferSize;
    CircularBuffer* buffer;
    float leftOver;
    bool toRemove;
    #ifdef MUSIC_VISU
    std::string name;
    #endif
};

std::map<MusicRef, MusicInfo> musics;
bool muted;
pthread_t oggDecompressionThread;
pthread_mutex_t mutex;
pthread_cond_t cond;
// map<filename, audio_compressed_content>
std::map<std::string, FileBuffer> name2buffer;

#ifdef MUSIC_VISU
std::map<Entity, std::pair<Entity, Entity> > visualisationEntities;
#endif

int decompressNextChunk(OggVorbis_File* file, int8_t* data, int chunkSize);
bool feed(OpaqueMusicPtr* ptr, MusicRef m, int forceCount, float dt);
OpaqueMusicPtr* startOpaque(MusicComponent* m, MusicRef r, MusicComponent* master, int offset);
void stopMusic(MusicComponent* m);
void clearAndRemoveInfo(MusicRef ref);
public:
MusicAPI* musicAPI;
AssetAPI* assetAPI;

void oggDecompRunLoop();
bool runDecompLoop;
};

