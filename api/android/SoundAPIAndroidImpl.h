#pragma once

#include "../SoundAPI.h"
#include <jni.h>

class SoundAPIAndroidImpl : public SoundAPI {
    public:
        SoundAPIAndroidImpl(JNIEnv *env, jobject assetMgr);

        void init();
        OpaqueSoundPtr* loadSound(const std::string& asset);
        void play(OpaqueSoundPtr* p, float volume);

    private:
        JNIEnv *env;
        jobject assetManager;
        struct SoundAPIAndroidImplData;
        SoundAPIAndroidImplData* datas;

};
