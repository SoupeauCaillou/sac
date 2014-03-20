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



#include "SoundAPILinuxOpenALImpl.h"

#include <AL/al.h>
#include <AL/alc.h>

#include <sstream>
#include <vector>
#include "base/Log.h"
#include "api/AssetAPI.h"

#include "util/OggDecoder.h"

static const char* errToString(ALenum err);
static void check_AL_errors(const char* context);
#define AL_OPERATION(x)  \
     (x); \
     check_AL_errors(#x);


struct OpenALOpaqueSoundPtr : public OpaqueSoundPtr {
    ALuint buffer;
};

SoundAPILinuxOpenALImpl::~SoundAPILinuxOpenALImpl() {
    AL_OPERATION(alDeleteSources(16, soundSources));
    delete[] soundSources;
}

void SoundAPILinuxOpenALImpl::init(AssetAPI* pAssetAPI, bool openALAlreadyInit) {
    assetAPI = pAssetAPI;

    if (!openALAlreadyInit) {
        ALCdevice* device = alcOpenDevice(0);
        ALCcontext* context = alcCreateContext(device, 0);
        if (!(device && context && alcMakeContextCurrent(context)))
            LOGW("probleme initialisation du son");
    }
    soundSources = new ALuint[16];
    // open al init is done earlier by MusicAPI
    AL_OPERATION(alGenSources(16, soundSources));
}

OpaqueSoundPtr* SoundAPILinuxOpenALImpl::loadSound(const std::string& asset) {
    FileBufferWithCursor fbc(assetAPI->loadAsset(asset));
    if (fbc.size == 0) {
        LOGW("Cannot read sound file: '" << asset << "'");
        return 0;
    }

    short* ptr = 0;
    OggInfo::Values info;
    int samples = OggDecoder::decode(fbc, &ptr, info);

    if (samples <= 0) {
        LOGW("Failed loading sound file: '" << asset << "'");
        delete[] fbc.data;
        return 0;
    }

    OpenALOpaqueSoundPtr* out = new OpenALOpaqueSoundPtr();
    AL_OPERATION(alGenBuffers(1, &out->buffer))
    AL_OPERATION(alBufferData(out->buffer, AL_FORMAT_MONO16, ptr, samples * sizeof(short), info.sampleRate))

    delete[] ptr;
    delete[] fbc.data;

    return out;
}

bool SoundAPILinuxOpenALImpl::play(OpaqueSoundPtr* p, float volume) {
    OpenALOpaqueSoundPtr* ptr = static_cast<OpenALOpaqueSoundPtr*>(p);
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
    return false;
}

static void check_AL_errors(const char* context) {
    int maxIterations=10;
    ALenum error;
    bool err = false;
    while (((error = alGetError()) != AL_NO_ERROR) && maxIterations > 0) {
        LOGW("OpenAL error during '" << context << "' -> " << errToString(error));
        maxIterations--;
        err = true;
    }
    LOGF_IF(err, "OpenAL error");
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
