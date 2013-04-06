#include "NameInputAPIAndroidImpl.h"
#include "base/Log.h"
#include <iostream>

NameInputAPIAndroidImpl::NameInputAPIAndroidImpl() : JNIWrapper<jni_name_api::Enum>("net/damsy/soupeaucaillou/api/NameInputAPI", true) {
	declareMethod(jni_name_api::Show, "showPlayerNameUi", "()V");
    declareMethod(jni_name_api::Done, "queryPlayerName", "()Ljava/lang/String;");
}

void NameInputAPIAndroidImpl::show() {
    env->CallVoidMethod(instance, methods[jni_name_api::Show]);
}

bool NameInputAPIAndroidImpl::done(std::string& name) {
    jstring n = (jstring) env->CallObjectMethod(instance, methods[jni_name_api::Done]);
    if (n) {
        const char *mfile = env->GetStringUTFChars(n, 0);
        LOGI("name choosen: '" << mfile << "'")
        name = mfile;
        env->ReleaseStringUTFChars(n, mfile);
        return true;
    }
    return false;
}

void NameInputAPIAndroidImpl::hide() {

}
