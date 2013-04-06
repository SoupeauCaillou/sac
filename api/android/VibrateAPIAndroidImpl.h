#pragma once

#include "../VibrateAPI.h"
#include "JNIWrapper.h"

namespace jni_vibrate_api {
    enum Enum {
        Vibrate
    };
}

class VibrateAPIAndroidImpl : public VibrateAPI, public JNIWrapper<jni_vibrate_api::Enum> {
    public:
        VibrateAPIAndroidImpl();

        void vibrate(float duration);
};
