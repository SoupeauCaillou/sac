/*!
 * \file MusicSystem.h
 * \brief 
 * \author Pierre-Eric Pelloux-Prayer
 * \author Gautier Pelloux-Prayer
 */
#pragma once

struct OggVorbis_File;
#include "System.h"
#include "../api/AssetAPI.h"
#include "../api/MusicAPI.h"
#include <pthread.h>

typedef int MusicRef;

#define InvalidMusicRef -1
class CircularBuffer;

#ifdef EMSCRIPTEN
#include <SDL/SDL_mixer.h>
#endif

// #define MUSIC_VISU

namespace MusicControl {
    enum Enum {
        Play,
        Stop,
        Pause
    };
}

/*! \struct MusicComponent
 *  \brief ? */
struct MusicComponent {
	MusicComponent() : music(InvalidMusicRef), loopNext(InvalidMusicRef), previousEnding(InvalidMusicRef), master(0), positionI(0), volume(1), control(MusicControl::Stop) {
		opaque[0] = opaque[1] = 0;
		fadeOut = fadeIn = 0;
        paused = false;
	}

    MusicRef music, loopNext; //!< ?
    MusicRef previousEnding; //!< ?
    MusicComponent* master; //!< ?
    float loopAt; //!< sec 
    int positionI; //!< in [0,1]
    #ifdef MUSIC_VISU
    float positionF; //!< ?
    #endif
    float fadeOut, fadeIn; //!< sec
    float volume; //!< ?
    float currentVolume; //!< handled by system - do not modify
    bool looped, paused; //!< ?
    MusicControl::Enum control; //!< ?

    struct OpaqueMusicPtr* opaque[2]; //!< 2 opaque structure to allow overlapping looping
};

#define theMusicSystem MusicSystem::GetInstance()
#define MUSIC(e) theMusicSystem.Get(e)

/*! \class Music
 *  \brief */
UPDATABLE_SYSTEM(Music)

public:
~MusicSystem();

/*! \brief ? */
void init();

/*! \brief ? */
bool isMuted() const { return muted; }

/*! \brief
 *  \param assetName
 *  \return */
MusicRef loadMusicFile(const std::string& assetName);

/*! \brief
 *  \param ref */
void unloadMusic(MusicRef ref);

/*! \brief
 *  \param enable */
void toggleMute(bool enable);

private:
/* textures cache */
MusicRef nextValidRef; //!< ?

#ifndef EMSCRIPTEN
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
#else
std::map<MusicRef, Mix_Chunk*> musics;
#endif

bool muted;
#ifndef EMSCRIPTEN
pthread_t oggDecompressionThread;
pthread_mutex_t mutex;
pthread_cond_t cond;
// map<filename, audio_compressed_content>
std::map<std::string, FileBuffer> name2buffer;
#endif

#ifdef MUSIC_VISU
std::map<Entity, std::pair<Entity, Entity> > visualisationEntities;
#endif

#ifndef EMSCRIPTEN
int decompressNextChunk(OggVorbis_File* file, int8_t* data, int chunkSize);
bool feed(OpaqueMusicPtr* ptr, MusicRef m, int forceCount, float dt);
#endif

/*! \brief
 *  \param m
 *  \param r
 *  \param master
 *  \param offset
 *  \return */
OpaqueMusicPtr* startOpaque(MusicComponent* m, MusicRef r, MusicComponent* master, int offset);

/*! \brief
 *  \param m */
void stopMusic(MusicComponent* m);

/*! \brief
 *  \param ref */
void clearAndRemoveInfo(MusicRef ref);
public:
MusicAPI* musicAPI; //!< 
AssetAPI* assetAPI; //!<

/*! \brief */
void oggDecompRunLoop();

bool runDecompLoop; //!<
};

