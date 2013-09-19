/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "CommunicationAPIAndroidImpl.h"

CommunicationAPIAndroidImpl::CommunicationAPIAndroidImpl()  : JNIWrapper<jni_comm_api::Enum>("net/damsy/soupeaucaillou/api/CommunicationAPI", true) {
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
