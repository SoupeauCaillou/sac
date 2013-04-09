#pragma once

#include "../LocalizeAPI.h"
#include "JNIWrapper.h"

namespace jni_loc_api {
    enum Enum {
        Text
    };
}

class LocalizeAPIAndroidImpl : public LocalizeAPI, public JNIWrapper<jni_loc_api::Enum> {
	public:
		LocalizeAPIAndroidImpl();
		std::string text(const std::string& s);

    private:
        std::map<std::string, std::string> cache;
};
