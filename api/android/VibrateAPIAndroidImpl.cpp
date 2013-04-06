#include "VibrateAPIAndroidImpl.h"
#include "base/Log.h"
#include <string>

VibrateAPIAndroidImpl::VibrateAPIAndroidImpl() : JNIWrapper<jni_vibrate_api::Enum>("net/damsy/soupeaucaillou/api/VibrateAPI", true) {
    declareMethod(jni_vibrate_api::Vibrate, "vibrate", "(F)V");
}

void VibrateAPIAndroidImpl::vibrate(float duration) {
    env->CallVoidMethod(instance, methods[jni_vibrate_api::Vibrate], duration);
}
