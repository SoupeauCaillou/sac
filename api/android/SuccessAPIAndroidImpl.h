#pragma once

#include "../SuccessAPI.h"
#include "JNIWrapper.h"

namespace jni_success_api {
    enum Enum {
        SuccessCompleted,
        OpenLeaderboard,
        OpenDashboard,
    };
}

class SuccessAPIAndroidImpl : public SuccessAPI, public JNIWrapper<jni_success_api::Enum> {
	public:
        SuccessAPIAndroidImpl();

		void successCompleted(const char* description, unsigned long successId);
        void openLeaderboard(int mode, int diff);
        void openDashboard();
};
