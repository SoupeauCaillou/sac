#include "SuccessAPIAndroidImpl.h"

void SuccessAPIAndroidImpl::init(JNIEnv *pEnv) {
	env = pEnv;	
}

void SuccessAPIAndroidImpl::uninit() {
	
}
		
void SuccessAPIAndroidImpl::successCompleted(const char* description, unsigned long successId) {
	SuccessAPI::successCompleted(description, successId);
	// android spec stuff
	jclass c = env->FindClass("net/damsy/soupeaucaillou/api/SuccessAPI");
	jmethodID mid = (env->GetStaticMethodID(c, "unlockAchievement", "(I)V"));
	int sid = (int) successId;
	env->CallStaticVoidMethod(c, mid, sid);
}

void SuccessAPIAndroidImpl::openLeaderboard(int mode, int diff) {
	jclass c = env->FindClass("net/damsy/soupeaucaillou/api/SuccessAPI");
	jmethodID mid = env->GetStaticMethodID(c, "openLeaderboard", "(II)V");
	env->CallStaticVoidMethod(c, mid, mode, diff);
}

void SuccessAPIAndroidImpl::openDashboard() {
	jclass c = env->FindClass("net/damsy/soupeaucaillou/api/SuccessAPI");
	jmethodID mid = env->GetStaticMethodID(c, "openDashboard", "()V");
	env->CallStaticVoidMethod(c, mid);
}