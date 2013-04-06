#pragma once

#include "../SoundAPI.h"
#include "JNIWrapper.h"

namespace jni_sound_api {
    enum Enum {
        LoadSound,
        Play
    };
}

class SoundAPIAndroidImpl : public SoundAPI, public JNIWrapper<jni_sound_api::Enum> {
    public:
        SoundAPIAndroidImpl();

        OpaqueSoundPtr* loadSound(const std::string& asset);
        bool play(OpaqueSoundPtr* p, float volume);
};
