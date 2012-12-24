#include "NameInputAPIAndroidImpl.h"
#include "base/Log.h"
#include <iostream>

static jmethodID jniMethodLookup(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
    jmethodID mId = env->GetStaticMethodID(c, name.c_str(), signature.c_str());
    if (!mId) {
        LOGW("JNI Error : could not find method '%s'/'%s'", name.c_str(), signature.c_str());
    }
    return mId;
}


struct NameInputAPIAndroidImpl::NameInputAPIAndroidImplDatas {
    jclass javaNameApi;
    jmethodID show;
    jmethodID hide;
    jmethodID done;
    bool initialized;
};

NameInputAPIAndroidImpl::NameInputAPIAndroidImpl() {
	datas = new NameInputAPIAndroidImplDatas();
	datas->initialized = false;
}

NameInputAPIAndroidImpl::~NameInputAPIAndroidImpl() {
    uninit();
    delete datas;
}

void NameInputAPIAndroidImpl::uninit() {
	if (datas->initialized) {
		env->DeleteGlobalRef(datas->javaNameApi);
		datas->initialized = false;
	}
}

void NameInputAPIAndroidImpl::init(JNIEnv* pEnv) {
    env = pEnv;

    datas->javaNameApi = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/api/NameInputAPI"));
    datas->show = jniMethodLookup(env, datas->javaNameApi, "showPlayerNameUi", "()V");
    datas->done = jniMethodLookup(env, datas->javaNameApi, "queryPlayerName", "()Ljava/lang/String;");
    // datas->hide = jniMethodLookup(env, datas->javaNameApi, "", "");
    datas->initialized = true;
}

void NameInputAPIAndroidImpl::show() {
    env->CallStaticVoidMethod(datas->javaNameApi, datas->show);
}

bool NameInputAPIAndroidImpl::done(std::string& name) {
    jstring n = (jstring) env->CallStaticObjectMethod(datas->javaNameApi, datas->done);
    if (n) {
        const char *mfile = env->GetStringUTFChars(n, 0);
        LOGW("name choosen: %s", mfile);
        name = mfile;
        env->ReleaseStringUTFChars(n, mfile);
        return true;
    }
    return false;
}

void NameInputAPIAndroidImpl::hide() {

}
