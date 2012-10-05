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
#include "SoundAPILinuxOpenALImpl.h"
#ifndef EMSCRIPTEN
#ifdef ANDROID
#include "tremor/ivorbisfile.h"
#else
#include "vorbis/vorbisfile.h"
#endif
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include <sstream>
#include <vector>
#include <cassert>
#include "base/Log.h"

#ifndef EMSCRIPTEN
static const char* errToString(ALenum err);
static void check_AL_errors(const char* context);
#define AL_OPERATION(x)  \
     (x); \
     check_AL_errors(#x);
#else
#define AL_OPERATION(x) 

#endif

struct OpenALOpaqueSoundPtr : public OpaqueSoundPtr {
#ifndef EMSCRIPTEN
    ALuint buffer;
#else
	Mix_Chunk* sample;
#endif
};

void SoundAPILinuxOpenALImpl::init() {
	#ifndef EMSCRIPTEN
    // open al init is done earlier by MusicAPI
    AL_OPERATION(alGenSources(16, soundSources));
	#else
	int ret = Mix_OpenAudio(0, 0, 0, 0);
	if (ret != 0) {
		LOGE("Mix_OpenAudio failed: %d", ret);
	}
	#endif
}

OpaqueSoundPtr* SoundAPILinuxOpenALImpl::loadSound(const std::string& asset) {
    std::stringstream a;
#ifdef DATADIR
	a << DATADIR;
#else
	a << "./assets/";
#endif
    a << asset;
   
#ifndef EMSCRIPTEN
    std::string s = a.str();
    const char* nm = s.c_str();
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
	    #ifdef ANDROID
        int n = ov_read(&vf, (char*)&data[readCount], sizeInBytes, &bitstream);
        #else
        int n = ov_read(&vf, (char*)&data[readCount], sizeInBytes, 0, 2, 1, &bitstream);
        #endif
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
#else
	OpenALOpaqueSoundPtr* out = new OpenALOpaqueSoundPtr();
	out->sample = Mix_LoadWAV(a.str().c_str());
	if (out->sample == 0) {
		LOGE("Cannot load %s", a.str().c_str());
	}
	return out;
#endif
    return out;
}

bool SoundAPILinuxOpenALImpl::play(OpaqueSoundPtr* p, float volume) {
    OpenALOpaqueSoundPtr* ptr = static_cast<OpenALOpaqueSoundPtr*>(p);
#ifndef EMSCRIPTEN
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