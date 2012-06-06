#include "AdAPIAndroidImpl.h"
#include "base/Log.h"
#include <string>

static jmethodID jniMethodLookup(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
    jmethodID mId = env->GetStaticMethodID(c, name.c_str(), signature.c_str());
    if (!mId) {
        LOGW("JNI Error : could not find method '%s'/'%s'", name.c_str(), signature.c_str());
    }
    return mId;
}

struct AdAPIAndroidImpl::AdAPIAndroidImplData {
 jclass cls;
 jmethodID showAd;
 jmethodID done;
};

AdAPIAndroidImpl::~AdAPIAndroidImpl() {
    env->DeleteGlobalRef(datas->cls);
    delete datas;
}

void AdAPIAndroidImpl::init(JNIEnv* pEnv) {
    env = pEnv;
    datas = new AdAPIAndroidImplData();

    datas->cls = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/heriswap/HeriswapJNILib"));
    datas->showAd = jniMethodLookup(env, datas->cls, "showAd", "()V");
    datas->done = jniMethodLookup(env, datas->cls, "done", "()Z");
}

void AdAPIAndroidImpl::showAd() {
    env->CallStaticObjectMethod(datas->cls, datas->showAd);
}

bool AdAPIAndroidImpl::done() {
    env->CallStaticBooleanMethod(datas->cls, datas->done);
}

