#pragma once

#include "../ExitAPI.h"
#include <jni.h>

class ExitAPIAndroidImpl : public ExitAPI {
	public:
		ExitAPIAndroidImpl();
		void init(JNIEnv *env);
		void uninit();
		void exitGame();

    	JNIEnv *env;
    private:
    	struct ExitAPIAndroidImplData;
		ExitAPIAndroidImplData* datas;
};
