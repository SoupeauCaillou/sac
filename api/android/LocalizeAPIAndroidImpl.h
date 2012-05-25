#pragma once

#include "../LocalizeAPI.h"
#include <jni.h>

class LocalizeAPIAndroidImpl : public LocalizeAPI {
	public:
		LocalizeAPIAndroidImpl(JNIEnv *env);
        ~LocalizeAPIAndroidImpl();
		void init();
		std::string text(const std::string& s, const std::string& spc);
	
    	JNIEnv *env;
    private:
    	struct LocalizeAPIAndroidImplData;
		LocalizeAPIAndroidImplData* datas;
};
