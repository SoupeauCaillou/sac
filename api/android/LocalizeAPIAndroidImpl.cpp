/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "LocalizeAPIAndroidImpl.h"
#include "base/Log.h"
#include <map>

static jmethodID jniMethodLookup(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
    jmethodID mId = env->GetStaticMethodID(c, name.c_str(), signature.c_str());
    if (!mId) {
        LOGW("JNI Error : could not find method '%s'/'%s'", name.c_str(), signature.c_str());
    }
    return mId;
}

struct LocalizeAPIAndroidImpl::LocalizeAPIAndroidImplData {	
	jclass javaLocApi;
	jmethodID localize;
	
	std::map<std::string, std::string> cache;
};

LocalizeAPIAndroidImpl::LocalizeAPIAndroidImpl(JNIEnv *pEnv) : env(pEnv) {
	
}

LocalizeAPIAndroidImpl::~LocalizeAPIAndroidImpl() {
    env->DeleteGlobalRef(datas->javaLocApi);
    delete datas;
}


void LocalizeAPIAndroidImpl::init() {
	datas = new LocalizeAPIAndroidImplData();
	
	datas->javaLocApi = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/heriswap/HeriswapJNILib"));
	datas->localize = jniMethodLookup(env, datas->javaLocApi, "localize", "(Ljava/lang/String;)Ljava/lang/String;");
}

std::string LocalizeAPIAndroidImpl::text(const std::string& s, const std::string& spc) {
	std::map<std::string, std::string>::iterator it = datas->cache.find(s);
	if (it != datas->cache.end()) {
		return it->second;
	}
	
	jstring name = env->NewStringUTF(s.c_str());
	jstring result = (jstring)env->CallStaticObjectMethod(datas->javaLocApi, datas->localize, name);
	
	const char *loc = env->GetStringUTFChars(result, 0);
	std::string b(loc);
	env->ReleaseStringUTFChars(result, loc);
	
	datas->cache[s] = b;
	return b;
}
