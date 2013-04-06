#include "AdAPIAndroidImpl.h"
#include "base/Log.h"
#include <string>


AdAPIAndroidImpl::AdAPIAndroidImpl() : JNIWrapper<jni_ad_api::Enum>("net/damsy/soupeaucaillou/api/AdAPI", true) {
    declareMethod(jni_ad_api::ShowAd, "showAd", "()Z");
    declareMethod(jni_ad_api::Done, "done", "()Z");
}

bool AdAPIAndroidImpl::showAd() {
    return env->CallBooleanMethod(instance, methods[jni_ad_api::ShowAd]);
}

bool AdAPIAndroidImpl::done() {
    return env->CallBooleanMethod(instance, methods[jni_ad_api::Done]);
}
