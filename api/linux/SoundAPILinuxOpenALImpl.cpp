#include "SoundAPILinuxOpenALImpl.h"
#include "tremor/ivorbisfile.h"
#include <AL/al.h>
#include <AL/alc.h>
#include <sstream>
#include <vector>
#include <cassert>
#include "base/Log.h"

static const char* errToString(ALenum err);
static void check_AL_errors(const char* context);
#define AL_OPERATION(x)  \
     (x); \
     check_AL_errors(#x);


struct OpenALOpaqueSoundPtr : public OpaqueSoundPtr {
    ALuint buffer;
};

void SoundAPILinuxOpenALImpl::init() {
    // open al init is done earlier by MusicAPI
    AL_OPERATION(alGenSources(16, soundSources));
}

OpaqueSoundPtr* SoundAPILinuxOpenALImpl::loadSound(const std::string& asset) {
    std::stringstream a;
    a << "./assets/" << asset;
    const char* nm = a.str().c_str();
    FILE* fd = fopen(nm, "rb");
    if (!fd) {
        LOGW("Cannot open %s", nm);
        return 0;
    }
    OggVorbis_File vf;
    if (ov_open(fd, &vf, 0, 0)) {
        LOGW("Failed loading: %s", nm);
        return 0;
    }
    int bitstream;
    int sizeInBytes = ov_pcm_total(&vf, -1) * 2;
    int8_t* data = new int8_t[sizeInBytes];
    int readCount = 0;
    do {
        int n = ov_read(&vf, (char*)&data[readCount], sizeInBytes, &bitstream);
        if (n == 0)
            break;
        readCount += n;
    } while (true);
    LOGW("%d != %d", readCount, sizeInBytes);
    assert(readCount == sizeInBytes);

    OpenALOpaqueSoundPtr* out = new OpenALOpaqueSoundPtr();
    AL_OPERATION(alGenBuffers(1, &out->buffer))
    AL_OPERATION(alBufferData(out->buffer, AL_FORMAT_MONO16, data, sizeInBytes, ov_info(&vf, -1)->rate))
    
    delete[] data;

    ov_clear(&vf);
    return out;
}

void SoundAPILinuxOpenALImpl::play(OpaqueSoundPtr* p, float volume) {
    OpenALOpaqueSoundPtr* ptr = static_cast<OpenALOpaqueSoundPtr*>(p);
    for (int i=0; i<16; i++) {
        int state;
        AL_OPERATION(alGetSourcei(soundSources[i], AL_SOURCE_STATE, &state))
        if (state != AL_PLAYING) {
            AL_OPERATION(alSourcei(soundSources[i], AL_BUFFER, ptr->buffer))
            AL_OPERATION(alSourcef(soundSources[i], AL_GAIN, volume))
            AL_OPERATION(alSourcePlay(soundSources[i]))
            return;
        }
    }
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
