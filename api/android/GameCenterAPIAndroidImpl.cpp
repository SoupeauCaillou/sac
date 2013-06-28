#include "GameCenterAPIAndroidImpl.h"

GameCenterAPIAndroidImpl::GameCenterAPIAndroidImpl()  : JNIWrapper<jni_gamecenter_api::Enum>("net/damsy/soupeaucaillou/api/GameCenterAPI", true) {
    declareMethod(jni_gamecenter_api::isRegistered, "isRegistered", "()Z");
    declareMethod(jni_gamecenter_api::isConnected, "isConnected", "()Z");
    declareMethod(jni_gamecenter_api::connectOrRegister, "connectOrRegister", "()V");
    declareMethod(jni_gamecenter_api::unlockAchievement, "unlockAchievement", "(I)V");
    declareMethod(jni_gamecenter_api::openAchievement, "openAchievement", "()V");
    declareMethod(jni_gamecenter_api::openLeaderboards, "openLeaderboards", "()V");
    declareMethod(jni_gamecenter_api::openSpecificLeaderboard, "openSpecificLeaderboard", "(I)V");
    declareMethod(jni_gamecenter_api::openDashboard, "openDashboard", "()V");
}

void GameCenterAPIAndroidImpl::init(JNIEnv* pEnv) {
    env = pEnv;
}

bool GameCenterAPIAndroidImpl::isRegistered() {
   return env->CallBooleanMethod(instance, methods[jni_gamecenter_api::isRegistered]);
}
bool GameCenterAPIAndroidImpl::isConnected() {
   return env->CallBooleanMethod(instance, methods[jni_gamecenter_api::isConnected]);
}
void GameCenterAPIAndroidImpl::connectOrRegister() {
   env->CallVoidMethod(instance, methods[jni_gamecenter_api::connectOrRegister]);
}
void GameCenterAPIAndroidImpl::unlockAchievement(int id) {
   env->CallVoidMethod(instance, methods[jni_gamecenter_api::unlockAchievement], id);
}
void GameCenterAPIAndroidImpl::openAchievement() {
   env->CallVoidMethod(instance, methods[jni_gamecenter_api::openAchievement]);
}
void GameCenterAPIAndroidImpl::openLeaderboards() {
   env->CallVoidMethod(instance, methods[jni_gamecenter_api::openLeaderboards]);
}
void GameCenterAPIAndroidImpl::openSpecificLeaderboard(int id) {
   env->CallVoidMethod(instance, methods[jni_gamecenter_api::openSpecificLeaderboard], id);
}
void GameCenterAPIAndroidImpl::openDashboard() {
    LOGI("opendash board");
    LOGE_IF(!env, "env is null!");
   env->CallVoidMethod(instance, methods[jni_gamecenter_api::openDashboard]);
}
