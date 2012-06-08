#pragma once

#include "../ExitAPI.h"
#include <jni.h>

class ExitAPIAndroidImpl : public ExitAPI {
	public:
		void init(JNIEnv *env);
		void exitGame();
	
    	JNIEnv *env;
    private:
    	struct ExitAPIAndroidImplData;
		ExitAPIAndroidImplData* datas;
};
