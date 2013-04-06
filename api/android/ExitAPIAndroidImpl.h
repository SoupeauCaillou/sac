#pragma once

#include "../ExitAPI.h"
#include "JNIWrapper.h"

namespace jni_exit_api {
    enum Enum {
        Exit
    };
}

class ExitAPIAndroidImpl : public ExitAPI, public JNIWrapper<jni_exit_api::Enum> {
	public:
		ExitAPIAndroidImpl();
		void exitGame();
};
