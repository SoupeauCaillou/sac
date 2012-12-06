#include "VibrateAPIAndroidImpl.h"
#include "base/Log.h"
#include <string>

static jmethodID jniMethodLookup(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
    jmethodID mId = env->GetStaticMethodID(c, name.c_str(), signature.c_str());
    if (!mId) {
        LOGW("JNI Error : could not find method '%s'/'%s'", name.c_str(), signature.c_str());
    }
    return mId;
}

struct VibrateAPIAndroidImpl::VibrateAPIAndroidImplData {
 jclass cls;
 jmethodID vibrate;
 bool initialized;
};

VibrateAPIAndroidImpl::VibrateAPIAndroidImpl() {
 datas = new VibrateAPIAndroidImplData();
 datas->initialized = false;
}

VibrateAPIAndroidImpl::~VibrateAPIAndroidImpl() {
 uninit();
    delete datas;
}

void VibrateAPIAndroidImpl::init(JNIEnv* pEnv) {
    env = pEnv;

    datas->cls = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/api/VibrateAPI"));
    datas->vibrate = jniMethodLookup(env, datas->cls, "vibrate", "(F)V");

    datas->initialized = true;
}

void VibrateAPIAndroidImpl::uninit() {
 if (datas->initialized) {
     env->DeleteGlobalRef(datas->cls);
     datas->initialized = false;
 }
}

void VibrateAPIAndroidImpl::vibrate(float duration) {
    env->CallStaticVoidMethod(datas->cls, datas->vibrate, duration);
}
