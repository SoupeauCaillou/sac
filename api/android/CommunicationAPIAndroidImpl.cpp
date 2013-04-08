#include "CommunicationAPIAndroidImpl.h"

CommunicationAPIAndroidImpl::CommunicationAPIAndroidImpl()  : JNIWrapper<jni_comm_api::Enum>("net/damsy/soupeaucaillou/api/CommunicationAPI", true) {
    declareMethod(jni_comm_api::IsGameCenterLoggedIn, "isGameCenterLoggedIn", "()Z");
    declareMethod(jni_comm_api::OpenGameCenter, "openGameCenter", "()V");
    declareMethod(jni_comm_api::GiftizMissionDone, "giftizMissionDone", "()V");
    declareMethod(jni_comm_api::GiftizGetButtonState, "giftizGetButtonState", "()I");
    declareMethod(jni_comm_api::GiftizButtonClicked, "giftizButtonClicked", "()V");
    declareMethod(jni_comm_api::ShareFacebook, "shareFacebook", "()V");
    declareMethod(jni_comm_api::ShareTwitter, "shareTwitter", "()V");
    declareMethod(jni_comm_api::MustShowRateDialog, "mustShowRateDialog", "()Z");
    declareMethod(jni_comm_api::RateItNow, "rateItNow", "()V");
    declareMethod(jni_comm_api::RateItLater, "rateItLater", "()V");
    declareMethod(jni_comm_api::RateItNever, "rateItNever", "()V");
}

void CommunicationAPIAndroidImpl::init(JNIEnv* pEnv) {
    env = pEnv;
}

bool CommunicationAPIAndroidImpl::isGameCenterLoggedIn() {
   return env->CallBooleanMethod(instance, methods[jni_comm_api::IsGameCenterLoggedIn]);
}

bool CommunicationAPIAndroidImpl::openGameCenter() {
   env->CallVoidMethod(instance, methods[jni_comm_api::OpenGameCenter]);
   return true;
}

void CommunicationAPIAndroidImpl::giftizMissionDone() {
   env->CallVoidMethod(instance, methods[jni_comm_api::GiftizMissionDone]);
}

int CommunicationAPIAndroidImpl::giftizGetButtonState() {
   return env->CallIntMethod(instance, methods[jni_comm_api::GiftizGetButtonState]);
}

void CommunicationAPIAndroidImpl::giftizButtonClicked() {
   env->CallVoidMethod(instance, methods[jni_comm_api::GiftizButtonClicked]);
}

void CommunicationAPIAndroidImpl::shareFacebook() {
   LOGI("share facebook !");
   env->CallVoidMethod(instance, methods[jni_comm_api::ShareFacebook]);
}

void CommunicationAPIAndroidImpl::shareTwitter() {
   LOGI("share twitter !");
   env->CallVoidMethod(instance, methods[jni_comm_api::ShareTwitter]);
}


bool CommunicationAPIAndroidImpl::mustShowRateDialog(){
   return env->CallBooleanMethod(instance, methods[jni_comm_api::MustShowRateDialog]);
}

void CommunicationAPIAndroidImpl::rateItNow(){
   env->CallVoidMethod(instance, methods[jni_comm_api::RateItNow]);
}

void CommunicationAPIAndroidImpl::rateItLater(){
   env->CallVoidMethod(instance, methods[jni_comm_api::RateItLater]);
}

void CommunicationAPIAndroidImpl::rateItNever(){
   env->CallVoidMethod(instance, methods[jni_comm_api::RateItNever]);
}
