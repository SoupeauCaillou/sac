#include "CommunicationAPIAndroidImpl.h"

#include <string>

#include "../../base/Log.h"

static jmethodID jniMethodLookup(JNIEnv* env, jclass c, const std::string& name, const std::string& signature) {
   jmethodID mId = env->GetStaticMethodID(c, name.c_str(), signature.c_str());
   if (!mId) {
      LOGW("JNI Error : could not find method '%s'/'%s'", name.c_str(), signature.c_str());
   }
   return mId;
}

struct CommunicationAPIAndroidImpl::CommunicationAPIAndroidImplDatas {
   jclass cls;

   jmethodID swarmInstalled;
   jmethodID swarmRegistering;
   jmethodID giftizMissionDone;
   jmethodID giftizGetButtonState;
   jmethodID giftizButtonClicked;
   jmethodID shareFacebook;
   jmethodID shareTwitter;
   jmethodID mustShowRateDialog;
   jmethodID rateItNow;
   jmethodID rateItLater;
   jmethodID rateItNever;

   bool initialized;
};

CommunicationAPIAndroidImpl::CommunicationAPIAndroidImpl() {
   datas = new CommunicationAPIAndroidImplDatas();
   datas->initialized = false;
}

CommunicationAPIAndroidImpl::~CommunicationAPIAndroidImpl() {
   if (datas->initialized) {
      env->DeleteGlobalRef(datas->cls);
   }
   delete datas;
}

void CommunicationAPIAndroidImpl::init(JNIEnv* pEnv) {
   if (datas->initialized) {
      LOGW("CommunicationAPI not properly uninitialized");
   }
   env = pEnv;

   datas->cls = (jclass)env->NewGlobalRef(env->FindClass("net/damsy/soupeaucaillou/api/CommunicationAPI"));

   datas->swarmInstalled = jniMethodLookup(env, datas->cls, "swarmEnabled", "()Z");
   datas->swarmRegistering = jniMethodLookup(env, datas->cls, "swarmRegistering", "()V");

   datas->giftizMissionDone = jniMethodLookup(env, datas->cls, "giftizMissionDone", "()V");
   datas->giftizGetButtonState = jniMethodLookup(env, datas->cls, "giftizGetButtonState", "()I");
   datas->giftizButtonClicked = jniMethodLookup(env, datas->cls, "giftizButtonClicked", "()V");

   datas->shareFacebook = jniMethodLookup(env, datas->cls, "shareFacebook", "()V");
   datas->shareTwitter = jniMethodLookup(env, datas->cls, "shareTwitter", "()V");

   datas->mustShowRateDialog = jniMethodLookup(env, datas->cls, "mustShowRateDialog", "()Z");
   datas->rateItNow = jniMethodLookup(env, datas->cls, "rateItNow", "()V");
   datas->rateItLater = jniMethodLookup(env, datas->cls, "rateItLater", "()V");
   datas->rateItNever = jniMethodLookup(env, datas->cls, "rateItNever", "()V");

   datas->initialized = true;
}

void CommunicationAPIAndroidImpl::uninit() {
   if (datas->initialized) {
      env->DeleteGlobalRef(datas->cls);
      datas->initialized = false;
   }
}

bool CommunicationAPIAndroidImpl::swarmInstalled() {
   return env->CallStaticBooleanMethod(datas->cls, datas->swarmInstalled);
}

void CommunicationAPIAndroidImpl::swarmRegistering() {
   env->CallStaticBooleanMethod(datas->cls, datas->swarmRegistering);
}

void CommunicationAPIAndroidImpl::giftizMissionDone() {
   env->CallStaticVoidMethod(datas->cls, datas->giftizMissionDone);
}

int CommunicationAPIAndroidImpl::giftizGetButtonState() {
   return env->CallStaticIntMethod(datas->cls, datas->giftizGetButtonState);
}

void CommunicationAPIAndroidImpl::giftizButtonClicked() {
   env->CallStaticVoidMethod(datas->cls, datas->giftizButtonClicked);
}

void CommunicationAPIAndroidImpl::shareFacebook() {
   LOGI("share facebook !");
   env->CallStaticBooleanMethod(datas->cls, datas->shareFacebook);
}

void CommunicationAPIAndroidImpl::shareTwitter() {
   LOGI("share twitter !");
   env->CallStaticBooleanMethod(datas->cls, datas->shareTwitter);
}


bool CommunicationAPIAndroidImpl::mustShowRateDialog(){
   return env->CallStaticBooleanMethod(datas->cls, datas->mustShowRateDialog);
}

void CommunicationAPIAndroidImpl::rateItNow(){
   env->CallStaticBooleanMethod(datas->cls, datas->rateItNow);
}

void CommunicationAPIAndroidImpl::rateItLater(){
   env->CallStaticBooleanMethod(datas->cls, datas->rateItLater);
}

void CommunicationAPIAndroidImpl::rateItNever(){
   env->CallStaticBooleanMethod(datas->cls, datas->rateItNever);
}
