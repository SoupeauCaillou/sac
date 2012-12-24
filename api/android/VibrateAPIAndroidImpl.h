#pragma once

#include "../VibrateAPI.h"
#include <jni.h>

class VibrateAPIAndroidImpl : public VibrateAPI {
    public:
        VibrateAPIAndroidImpl();
        ~VibrateAPIAndroidImpl();

        void init(JNIEnv* env);
        void uninit();

        void vibrate(float duration);

    private:
        JNIEnv *env;
        struct VibrateAPIAndroidImplData;
        VibrateAPIAndroidImplData* datas;
};
