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

	bool initialized;
	std::map<std::string, std::string> cache;
};

LocalizeAPIAndroidImpl::LocalizeAPIAndroidImpl() {
	datas = new LocalizeAPIAndroidImplData();
	datas->initialized = false;
}

LocalizeAPIAndroidImpl::~LocalizeAPIAndroidImpl() {
    uninit();
    delete datas;
}


void LocalizeAPIAndroidImpl::init(JNIEnv *pEnv) {
	env = pEnv;

	datas->javaLocApi = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/api/LocalizeAPI"));
	datas->localize = jniMethodLookup(env, datas->javaLocApi, "localize", "(Ljava/lang/String;)Ljava/lang/String;");
	datas->initialized = true;
}

void LocalizeAPIAndroidImpl::uninit() {
	if (datas->initialized) {
		env->DeleteGlobalRef(datas->javaLocApi);
		datas->initialized = false;
	}
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

void LocalizeAPIAndroidImpl::changeLanguage(const std::string& s) {

}
