#include "SoundAPILinuxOpenALImpl.h"
#ifndef SAC_EMSCRIPTEN
#ifdef SAC_ANDROID
#include "tremor/ivorbisfile.h"
#else
#include "vorbis/vorbisfile.h"
#endif
#include <al.h>
#include <alc.h>
#endif
#include <sstream>
#include <vector>
#include <cassert>
#include "base/Log.h"
#include "api/AssetAPI.h"

#ifndef SAC_EMSCRIPTEN
static const char* errToString(ALenum err);
static void check_AL_errors(const char* context);
#define AL_OPERATION(x)  \
     (x); \
     check_AL_errors(#x);
#else
#define AL_OPERATION(x)

#endif

struct OpenALOpaqueSoundPtr : public OpaqueSoundPtr {
#ifndef SAC_EMSCRIPTEN
    ALuint buffer;
#else
	Mix_Chunk* sample;
#endif
};

SoundAPILinuxOpenALImpl::~SoundAPILinuxOpenALImpl() {
    #ifndef SAC_EMSCRIPTEN
    AL_OPERATION(alDeleteSources(16, soundSources));
    delete[] soundSources;
    #endif
}

void SoundAPILinuxOpenALImpl::init(AssetAPI* pAssetAPI) {
    assetAPI = pAssetAPI;
	#ifndef SAC_EMSCRIPTEN
    soundSources = new ALuint[16];
    // open al init is done earlier by MusicAPI
    AL_OPERATION(alGenSources(16, soundSources));
	#else
	int ret = Mix_OpenAudio(0, 0, 0, 0);
	if (ret != 0) {
		LOG(ERROR) << "Mix_OpenAudio failed: " <<ret;
	}
	#endif
}

OpaqueSoundPtr* SoundAPILinuxOpenALImpl::loadSound(const std::string& asset) {
#ifndef SAC_EMSCRIPTEN
    FileBufferWithCursor fbc(assetAPI->loadAsset(asset));
    if (fbc.size == 0) {
        LOGW("Cannot read sound file: '" << asset << "'")
        return 0;
    }
    ov_callbacks cb;
    cb.read_func = &FileBufferWithCursor::read_func;
    cb.seek_func = &FileBufferWithCursor::seek_func;
    cb.close_func = &FileBufferWithCursor::close_func;
    cb.tell_func = &FileBufferWithCursor::tell_func;
    OggVorbis_File vf;
    if (ov_open_callbacks(&fbc, &vf, 0, 0, cb)) {
        LOGW("Failed loading sound file: '" << asset << "'")
        delete[] fbc.data;
        return 0;
    }

    int bitstream;
    int sizeInBytes = ov_pcm_total(&vf, -1) * 2;
    int8_t* data = new int8_t[sizeInBytes];
    int readCount = 0;
    do {
	    #ifdef SAC_ANDROID
        int n = ov_read(&vf, (char*)&data[readCount], sizeInBytes, &bitstream);
        #else
        int n = ov_read(&vf, (char*)&data[readCount], sizeInBytes, 0, 2, 1, &bitstream);
        #endif
        if (n == 0)
            break;
        readCount += n;
    } while (true);
    LOGW_IF(readCount != sizeInBytes, "Weird byte count read: " << readCount << '/' << sizeInBytes)

    OpenALOpaqueSoundPtr* out = new OpenALOpaqueSoundPtr();
    AL_OPERATION(alGenBuffers(1, &out->buffer))
    AL_OPERATION(alBufferData(out->buffer, AL_FORMAT_MONO16, data, sizeInBytes, ov_info(&vf, -1)->rate))

    delete[] data;

    ov_clear(&vf);
    delete[] fbc.data;
#else
    std::stringstream a;
#ifdef SAC_ASSETS_DIR
    a << SAC_ASSETS_DIR;
#else
    a << "./assets/";
#endif
    a << asset;
	OpenALOpaqueSoundPtr* out = new OpenALOpaqueSoundPtr();
	out->sample = Mix_LoadWAV(a.str().c_str());
	if (out->sample == 0) {
		LOGW("Cannot load " << a.str())
	}
#endif
    return out;
}

bool SoundAPILinuxOpenALImpl::play(OpaqueSoundPtr* p, float volume) {
    OpenALOpaqueSoundPtr* ptr = static_cast<OpenALOpaqueSoundPtr*>(p);
#ifndef SAC_EMSCRIPTEN
    for (int i=0; i<16; i++) {
        int state;
        AL_OPERATION(alGetSourcei(soundSources[i], AL_SOURCE_STATE, &state))
        if (state != AL_PLAYING) {
            AL_OPERATION(alSourcei(soundSources[i], AL_BUFFER, ptr->buffer))
            AL_OPERATION(alSourcef(soundSources[i], AL_GAIN, volume))
            AL_OPERATION(alSourcePlay(soundSources[i]))
            return true;
        }
    }
#else
	Mix_PlayChannel(-1, ptr->sample, 1);
	Mix_Volume(-1, MIX_MAX_VOLUME);
#endif
    return false;
}

#ifndef SAC_EMSCRIPTEN
static void check_AL_errors(const char* context) {
    int maxIterations=10;
    ALenum error;
    bool err = false;
    while (((error = alGetError()) != AL_NO_ERROR) && maxIterations > 0) {
        LOGW("OpenAL error during '" << context << "' -> " << errToString(error))
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
