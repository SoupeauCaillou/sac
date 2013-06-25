#include "StringInputAPIAndroidImpl.h"
#include "base/Log.h"
#include <iostream>

StringInputAPIAndroidImpl::StringInputAPIAndroidImpl() : JNIWrapper<jni_name_api::Enum>("net/damsy/soupeaucaillou/api/StringInputAPI", true) {
	declareMethod(jni_name_api::AskUserInput, "showPlayerKeyboardUI", "()V");
    declareMethod(jni_name_api::Done, "askUserInput", "()Ljava/lang/String;");
    declareMethod(jni_name_api::CancelUserInput, "closePlayerKeyboardUI", "()V");
}

void StringInputAPIAndroidImpl::askUserInput(const std::string&, const int) {
    LOGT("Use parameters?");
    env->CallVoidMethod(instance, methods[jni_name_api::AskUserInput]);
}

bool StringInputAPIAndroidImpl::done(std::string& name) {
    jstring n = (jstring) env->CallObjectMethod(instance, methods[jni_name_api::Done]);
    const char *mfile = env->GetStringUTFChars(n, 0);
    name = mfile;
    if (n) {
        LOGI("Entered string: '" << mfile << "'");
        env->ReleaseStringUTFChars(n, mfile);
        return true;
    }
    return false;
}

void StringInputAPIAndroidImpl::cancelUserInput() {
    env->CallVoidMethod(instance, methods[jni_name_api::CancelUserInput]);
}
