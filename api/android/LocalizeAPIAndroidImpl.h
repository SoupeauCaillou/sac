#pragma once

#include "../LocalizeAPI.h"
#include <jni.h>

class LocalizeAPIAndroidImpl : public LocalizeAPI {
	public:
		LocalizeAPIAndroidImpl(JNIEnv *env);
		void init();
		std::string text(const std::string& s);
	
    	JNIEnv *env;
    private:
    	struct LocalizeAPIAndroidImplData;
		LocalizeAPIAndroidImplData* datas;
};
