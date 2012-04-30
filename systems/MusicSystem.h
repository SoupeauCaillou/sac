#pragma once

#include <map>

#include "base/MathUtil.h"
#include "base/Log.h"

#include "tremor/ivorbisfile.h"

#include "System.h"
#include "../api/AssetAPI.h"
#include "../api/MusicAPI.h"

typedef int MusicRef;

#define InvalidMusicRef -1

#define MUSIC_VISU

struct MusicComponent {
	MusicComponent() : music(InvalidMusicRef), loopNext(InvalidMusicRef), master(0), control(Stop) {
		opaque[0] = opaque[1] = 0;
	}

    MusicRef music, loopNext;
    MusicComponent* master;
    float loopAt; // sec
    int positionI; // in [0,1]
    float positionF;
    float fadeOut; // sec
    float volume;
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

MusicRef loadMusicFile(const std::string& assetName);

void toggleMute(bool enable);

private:
/* textures cache */
MusicRef nextValidRef;
std::map<std::string, MusicRef> assetSounds;

struct MusicInfo {
    OggVorbis_File* ovf;
    float totalTime;
    int nbSamples;
    int sampleRate;
};

std::map<MusicRef, MusicInfo> musics;
bool mute;

std::map<std::string, FileBuffer> name2buffer;

#ifdef MUSIC_VISU
std::map<Entity, Entity> visualisationEntities;
#endif

int decompressNextChunk(OggVorbis_File* file, int8_t** data, int chunkSize);
bool feed(OpaqueMusicPtr* ptr, MusicRef m, int forceCount);
OpaqueMusicPtr* startOpaque(MusicComponent* m, MusicRef r, int offset);

public:
MusicAPI* musicAPI;
AssetAPI* assetAPI;
};

