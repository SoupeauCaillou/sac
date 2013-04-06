#include "ExitAPIAndroidImpl.h"
#include "base/Log.h"
#include <string>

ExitAPIAndroidImpl::ExitAPIAndroidImpl() : JNIWrapper<jni_exit_api::Enum>("net/damsy/soupeaucaillou/api/ExitAPI", true) {
	declareMethod(jni_exit_api::Exit, "exitGame", "()V");
}

void ExitAPIAndroidImpl::exitGame() {
	env->CallVoidMethod(instance, methods[jni_exit_api::Exit]);
}
