#pragma once

#include "../SuccessAPI.h"
#include <jni.h>

class SuccessAPIAndroidImpl : public SuccessAPI {
	public:
		void init(JNIEnv *env);
		void uninit();

		void successCompleted(const char* description, unsigned long successId);
        void openLeaderboard(int mode, int diff);
        void openDashboard();
	private:
        JNIEnv *env;
};
