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
#include "MusicAPILinuxOpenALImpl.h"

#ifndef EMSCRIPTEN
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <SDL/SDL_mixer.h>
#endif

#include <cassert>
#include <vector>
#include "../../base/Log.h"

#ifndef EMSCRIPTEN
static const char* errToString(ALenum err);
static void check_AL_errors(const char* context);
#define AL_OPERATION(x)  \
     (x); \
     check_AL_errors(#x);
#else
#define AL_OPERATION(x) 
#endif
#define MUSIC_CHUNK_SIZE(freq) SEC_TO_BYTE(0.5, freq)

struct OpenALOpaqueMusicPtr : public OpaqueMusicPtr {
	#ifndef EMSCRIPTEN
    ALuint source;
    std::vector<ALuint> queuedBuffers;
    int queuedSize;
    #else
    int channel;
    #endif
};

void MusicAPILinuxOpenALImpl::init() {
#ifndef EMSCRIPTEN
    ALCdevice* device = alcOpenDevice(NULL);
    ALCcontext* context = alcCreateContext(device, NULL);
    if (!(device && context && alcMakeContextCurrent(context)))
        LOGW("probleme initialisation du son");
#endif
}

OpaqueMusicPtr* MusicAPILinuxOpenALImpl::createPlayer(int sampleRate __attribute__((unused))) {
    OpenALOpaqueMusicPtr* result = new OpenALOpaqueMusicPtr();
    // create source
    AL_OPERATION(alGenSources(1, &result->source))
    return result;
}

int MusicAPILinuxOpenALImpl::pcmBufferSize(int sampleRate) {
    return SAMPLES_TO_BYTE(SEC_TO_SAMPLES(0.05, sampleRate), sampleRate);
}

int8_t* MusicAPILinuxOpenALImpl::allocate(int size) {
    return new int8_t[size];
}

void MusicAPILinuxOpenALImpl::deallocate(int8_t* b) {
    delete[] b;
}

int MusicAPILinuxOpenALImpl::initialPacketCount(OpaqueMusicPtr* ptr __attribute__((unused))) {
    return 10;
}

void MusicAPILinuxOpenALImpl::queueMusicData(OpaqueMusicPtr* ptr, int8_t* data, int size, int sampleRate) {
	OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
#ifndef EMSCRIPTEN
    // create buffer
    ALuint buffer;
    AL_OPERATION(alGenBuffers(1, &buffer))
    AL_OPERATION(alBufferData(buffer, AL_FORMAT_MONO16, data, size, sampleRate))

    AL_OPERATION(alSourceQueueBuffers(openalptr->source, 1, &buffer))
    openalptr->queuedBuffers.push_back(buffer);
    openalptr->queuedSize += size;
    delete[] data;
#else
	openalptr->channel = Mix_PlayChannel(-1, static_cast<Mix_Chunk*>((void*)data), 0);
#endif
}

void MusicAPILinuxOpenALImpl::startPlaying(OpaqueMusicPtr* ptr, OpaqueMusicPtr* master, int offset) {
#ifndef EMSCRIPTEN
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
    if (master) {
	    int pos;
	    AL_OPERATION(alGetSourcei((static_cast<OpenALOpaqueMusicPtr*>(master))->source, AL_SAMPLE_OFFSET, &pos))
	    setPosition(ptr, pos + offset);
    }
    AL_OPERATION(alSourcePlay(openalptr->source))
#endif
}

void MusicAPILinuxOpenALImpl::stopPlayer(OpaqueMusicPtr* ptr) {
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
#ifndef EMSCRIPTEN
    AL_OPERATION(alSourceStop(openalptr->source))
#else
	Mix_HaltChannel(openalptr->channel);
#endif
}

int MusicAPILinuxOpenALImpl::getPosition(OpaqueMusicPtr* ptr) {
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
    int pos = 0;
    AL_OPERATION(alGetSourcei(openalptr->source, AL_SAMPLE_OFFSET, &pos))
    return pos;
}

void MusicAPILinuxOpenALImpl::setPosition(OpaqueMusicPtr* ptr, int pos) {
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
    AL_OPERATION(alSourcei(openalptr->source, AL_SAMPLE_OFFSET, pos))
}

void MusicAPILinuxOpenALImpl::setVolume(OpaqueMusicPtr* ptr, float volume) {
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
#ifndef EMSCRIPTEN
    AL_OPERATION(alSourcef(openalptr->source, AL_GAIN, volume))
#else
	Mix_Volume(openalptr->channel, volume * MIX_MAX_VOLUME * 0.6);
#endif
}

bool MusicAPILinuxOpenALImpl::isPlaying(OpaqueMusicPtr* ptr) {
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
    #ifndef EMSCRIPTEN
    ALint state;
    AL_OPERATION(alGetSourcei(openalptr->source, AL_SOURCE_STATE, &state))
    return state == AL_PLAYING;
    #else
    return true;//Mix_Playing(openalptr->channel);
   	#endif
}

void MusicAPILinuxOpenALImpl::deletePlayer(OpaqueMusicPtr* ptr) {
#ifndef EMSCRIPTEN
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
    stopPlayer(ptr);
    // destroy buffers
    for (unsigned int i=0; i<openalptr->queuedBuffers.size(); i++) {
        AL_OPERATION(alSourceUnqueueBuffers(openalptr->source, 1, &openalptr->queuedBuffers[i]))
        AL_OPERATION(alDeleteBuffers(1, &openalptr->queuedBuffers[i]))
    }
    // destroy source
    AL_OPERATION(alDeleteSources(1, &openalptr->source))
#endif
    delete ptr; 
}

#ifndef EMSCRIPTEN
static void check_AL_errors(const char* context) {
    int maxIterations=10;
    ALenum error;
    bool err = false;
    while (((error = alGetError()) != AL_NO_ERROR) && maxIterations > 0) {
        LOGW("OpenAL error during '%s' -> %s", context, errToString(error));
        maxIterations--;
        err = true;
    }
    assert(!err);
}

static const char* errToString(ALenum err) {
    switch (err) {
    case AL_NO_ERROR: return "AL(No error)";
    case AL_INVALID_NAME: return "AL(Invalid name)";
    case AL_INVALID_VALUE: return "AL(Invalid value)";
    case AL_INVALID_ENUM: return "AL(Invalid enum)";
    case AL_INVALID_OPERATION: return "AL(Invalid operation)";
    case AL_OUT_OF_MEMORY: return "AL(Out of memory)";
    default: return "AL(Unknown)";
    }
}
#endif
