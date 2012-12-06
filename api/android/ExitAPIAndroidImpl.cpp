#include "ExitAPIAndroidImpl.h"
#include "base/Log.h"
#include <map>
#include <string>

static jmethodID jniMethodLookup(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
    jmethodID mId = env->GetStaticMethodID(c, name.c_str(), signature.c_str());
    if (!mId) {
        LOGW("JNI Error : could not find method '%s'/'%s'", name.c_str(), signature.c_str());
    }
    return mId;
}

struct ExitAPIAndroidImpl::ExitAPIAndroidImplData {
	jclass cls;
	jmethodID exitGame;
	bool initialized;
};

ExitAPIAndroidImpl::ExitAPIAndroidImpl() {
	datas = new ExitAPIAndroidImplData();
	datas->initialized = false;
}

void ExitAPIAndroidImpl::init(JNIEnv* pEnv) {
	env = pEnv;

	datas->cls = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/api/ExitAPI"));
	datas->exitGame = jniMethodLookup(env, datas->cls, "exitGame", "()V");
	datas->initialized = true;
}

void ExitAPIAndroidImpl::uninit() {
	if (datas->initialized) {
		env->DeleteGlobalRef(datas->cls);
		datas->initialized = false;
	}
}

void ExitAPIAndroidImpl::exitGame() {
	env->CallStaticVoidMethod(datas->cls, datas->exitGame);
}
