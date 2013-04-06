#pragma once

#include "../NameInputAPI.h"
#include "JNIWrapper.h"

namespace jni_name_api {
    enum Enum {
        Show,
        Done,
    };
}

class NameInputAPIAndroidImpl : public NameInputAPI, public JNIWrapper<jni_name_api::Enum> {
    public:
    	NameInputAPIAndroidImpl();

        void show();
        bool done(std::string& name);
        void hide();
};
