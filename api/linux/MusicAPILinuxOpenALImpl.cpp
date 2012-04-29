#include "MusicAPILinuxOpenALImpl.h"

#include <sndfile.h>
#include <AL/al.h>
#include <AL/alc.h>

#include <cassert>
#include <vector>
#include "../../base/Log.h"

static const char* errToString(ALenum err);
static void check_AL_errors(const char* context);
#define AL_OPERATION(x)  \
     (x); \
     check_AL_errors(#x);


struct OpenALOpaqueMusicPtr : public OpaqueMusicPtr {
    ALuint source;
    std::vector<ALuint> queuedBuffers;
    int queuedSize;
};

void MusicAPILinuxOpenALImpl::init() {
    ALCdevice* device = alcOpenDevice(NULL);
    ALCcontext* context = alcCreateContext(device, NULL);
    if (!(device && context && alcMakeContextCurrent(context)))
        LOGW("probleme initialisation du son");
}

OpaqueMusicPtr* MusicAPILinuxOpenALImpl::createPlayer() {
    OpenALOpaqueMusicPtr* result = new OpenALOpaqueMusicPtr();
    // create source
    AL_OPERATION(alGenSources(1, &result->source))
    return result;
}

void MusicAPILinuxOpenALImpl::queueMusicData(OpaqueMusicPtr* ptr, int8_t* data, int size, int sampleRate) {
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
    // create buffer
    ALuint buffer;
    AL_OPERATION(alGenBuffers(1, &buffer))
    AL_OPERATION(alBufferData(buffer, AL_FORMAT_MONO16, data, size, sampleRate))

    AL_OPERATION(alSourceQueueBuffers(openalptr->source, 1, &buffer))
    openalptr->queuedBuffers.push_back(buffer);
    openalptr->queuedSize += size;
    delete[] data;
}

bool MusicAPILinuxOpenALImpl::needData(OpaqueMusicPtr* ptr, int sampleRate) {
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
    int pos = getPosition(ptr);
    return ((pos - openalptr->queuedSize) < (0.5 * sampleRate * 2));
}

void MusicAPILinuxOpenALImpl::startPlaying(OpaqueMusicPtr* ptr) {
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
    AL_OPERATION(alSourcePlay(openalptr->source))
}

void MusicAPILinuxOpenALImpl::stopPlayer(OpaqueMusicPtr* ptr) {
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
    AL_OPERATION(alSourceStop(openalptr->source))
}

int MusicAPILinuxOpenALImpl::getPosition(OpaqueMusicPtr* ptr) {
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
    int pos;
    AL_OPERATION(alGetSourcei(openalptr->source, AL_BYTE_OFFSET, &pos))
    return pos;
}

void MusicAPILinuxOpenALImpl::setPosition(OpaqueMusicPtr* ptr, int pos) {
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
    AL_OPERATION(alSourcei(openalptr->source, AL_BYTE_OFFSET, pos))
}

void MusicAPILinuxOpenALImpl::setVolume(OpaqueMusicPtr* ptr, float volume) {
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
    AL_OPERATION(alSourcef(openalptr->source, AL_GAIN, volume))
}

bool MusicAPILinuxOpenALImpl::isPlaying(OpaqueMusicPtr* ptr) {
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
    ALint state;
    AL_OPERATION(alGetSourcei(openalptr->source, AL_SOURCE_STATE, &state))
    return state == AL_PLAYING;
}

void MusicAPILinuxOpenALImpl::deletePlayer(OpaqueMusicPtr* ptr) {
    OpenALOpaqueMusicPtr* openalptr = static_cast<OpenALOpaqueMusicPtr*> (ptr);
    stopPlayer(ptr);
    // destroy buffers
    for (int i=0; i<openalptr->queuedBuffers.size(); i++) {
        AL_OPERATION(alSourceUnqueueBuffers(openalptr->source, 1, &openalptr->queuedBuffers[i]))
        AL_OPERATION(alDeleteBuffers(1, &openalptr->queuedBuffers[i]))
    }
    // destroy source
    AL_OPERATION(alDeleteSources(1, &openalptr->source))
    delete ptr;
}

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

