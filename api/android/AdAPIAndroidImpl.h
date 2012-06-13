#pragma once

#include "../AdAPI.h"
#include <jni.h>

class AdAPIAndroidImpl : public AdAPI {
    public:
        ~AdAPIAndroidImpl();

        void init(JNIEnv* env);

        bool showAd();
        bool done();

    private:
        JNIEnv *env;
        struct AdAPIAndroidImplData;
        AdAPIAndroidImplData* datas;
};
