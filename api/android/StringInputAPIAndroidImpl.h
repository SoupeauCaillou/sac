#pragma once

#include "api/StringInputAPI.h"
#include "JNIWrapper.h"

namespace jni_name_api {
    enum Enum {
        AskUserInput,
        Done,
        CancelUserInput,
    };
}

class StringInputAPIAndroidImpl : public StringInputAPI, public JNIWrapper<jni_name_api::Enum> {
    public:
    	StringInputAPIAndroidImpl();

        void askUserInput(const std::string& initial, const int imaxSize);
        bool done(std::string & entry);
        void cancelUserInput();
};
