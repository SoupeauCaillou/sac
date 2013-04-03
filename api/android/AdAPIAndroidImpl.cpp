#include "AdAPIAndroidImpl.h"
#include "base/Log.h"
#include <string>

static jmethodID jniMethodLookup(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
    jmethodID mId = env->GetStaticMethodID(c, name.c_str(), signature.c_str());
    if (!mId) {
        LOGF("JNI Error : could not find method '" << name << "'/'" << signature << "'")
    }
    return mId;
}

struct AdAPIAndroidImpl::AdAPIAndroidImplData {
 jclass cls;
 jmethodID showAd;
 jmethodID done;
 bool initialized;
};

AdAPIAndroidImpl::AdAPIAndroidImpl() {
	datas = new AdAPIAndroidImplData();
	datas->initialized = false;
}

AdAPIAndroidImpl::~AdAPIAndroidImpl() {
	uninit();
    delete datas;
}

void AdAPIAndroidImpl::init(JNIEnv* pEnv) {
    env = pEnv;

    datas->cls = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/api/AdAPI"));
    datas->showAd = jniMethodLookup(env, datas->cls, "showAd", "()Z");
    datas->done = jniMethodLookup(env, datas->cls, "done", "()Z");

    datas->initialized = true;
}

void AdAPIAndroidImpl::uninit() {
	if (datas->initialized) {
    	env->DeleteGlobalRef(datas->cls);
    	datas->initialized = false;
	}
}

bool AdAPIAndroidImpl::showAd() {
    return env->CallStaticBooleanMethod(datas->cls, datas->showAd);
}

bool AdAPIAndroidImpl::done() {
    return env->CallStaticBooleanMethod(datas->cls, datas->done);
}

