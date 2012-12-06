#pragma once

#include "../SoundAPI.h"
#include <jni.h>

class SoundAPIAndroidImpl : public SoundAPI {
    public:
        SoundAPIAndroidImpl();
        ~SoundAPIAndroidImpl();

        void init(JNIEnv *env, jobject assetMgr);
		void uninit();

        OpaqueSoundPtr* loadSound(const std::string& asset);
        bool play(OpaqueSoundPtr* p, float volume);

    private:
        JNIEnv *env;
        jobject assetManager;
        struct SoundAPIAndroidImplData;
        SoundAPIAndroidImplData* datas;

};
