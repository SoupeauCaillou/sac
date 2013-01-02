#pragma once

#include "../LocalizeAPI.h"
#include <jni.h>

class LocalizeAPIAndroidImpl : public LocalizeAPI {
	public:
		LocalizeAPIAndroidImpl();
        ~LocalizeAPIAndroidImpl();
		void init(JNIEnv *env);
		void uninit();
		std::string text(const std::string& s, const std::string& spc);
		void changeLanguage(const std::string& s);

    	JNIEnv *env;
    private:
    	struct LocalizeAPIAndroidImplData;
		LocalizeAPIAndroidImplData* datas;
};
