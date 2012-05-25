#pragma once

#include "../NameInputAPI.h"
#include <jni.h>

class NameInputAPIAndroidImpl : public NameInputAPI {
    public:
        ~NameInputAPIAndroidImpl();
        void init(JNIEnv* env);
        void show();
        bool done(std::string& name);
        void hide();

    private:
        JNIEnv* env;
        class NameInputAPIAndroidImplDatas;
        NameInputAPIAndroidImplDatas* datas;
};