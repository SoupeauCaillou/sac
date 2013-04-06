#pragma once

#include "../AdAPI.h"
#include "JNIWrapper.h"

namespace jni_ad_api {
    enum Enum {
        ShowAd,
        Done,
    };
}

class AdAPIAndroidImpl : public AdAPI, public JNIWrapper<jni_ad_api::Enum> {
    public:
    	AdAPIAndroidImpl();

        bool showAd();
        bool done();
};
