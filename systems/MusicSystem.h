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
    MusicRef music, loopNext;
    float loopAt; // sec
    float position; // in [0,1]
    float fadeOut; // sec
    float volume;
    enum {
        Start, Stop
    } control;

    struct OpaqueMusicPtr* opaque;
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
std::map<MusicRef, OggVorbis_File*> musics;
bool mute;

std::map<std::string, FileBuffer> name2buffer;

#ifdef MUSIC_VISU
std::map<Entity, Entity> visualisationEntities;
#endif

int decompressNextChunk(OggVorbis_File* file, int8_t** data);

public:
MusicAPI* musicAPI;
AssetAPI* assetAPI;
};

