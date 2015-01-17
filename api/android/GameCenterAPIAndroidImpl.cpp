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



#include "GameCenterAPIAndroidImpl.h"
#include <vector>

std::vector<std::function<void (int)> > weeklyRankFunctions;

#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL Java_net_damsy_soupeaucaillou_api_GameCenterAPI_weeklyRank
  (JNIEnv *, jclass, jlong rank) {
    weeklyRankFunctions.back()(rank);
    weeklyRankFunctions.clear();
}
#ifdef __cplusplus
}
#endif

GameCenterAPIAndroidImpl::GameCenterAPIAndroidImpl()  : JNIWrapper<jni_gamecenter_api::Enum>("net/damsy/soupeaucaillou/api/GameCenterAPI", true) {
    declareMethod(jni_gamecenter_api::isRegistered, "isRegistered", "()Z");
    declareMethod(jni_gamecenter_api::isConnected, "isConnected", "()Z");
    declareMethod(jni_gamecenter_api::connectOrRegister, "connectOrRegister", "()V");
    declareMethod(jni_gamecenter_api::disconnect, "disconnect", "()V");

    declareMethod(jni_gamecenter_api::unlockAchievement, "unlockAchievement", "(I)V");
    declareMethod(jni_gamecenter_api::updateAchievementProgression, "updateAchievementProgression", "(II)V");

    declareMethod(jni_gamecenter_api::submitScore, "submitScore", "(ILjava/lang/String;)V");
    declareMethod(jni_gamecenter_api::getWeeklyRank, "getWeeklyRank", "(I)V");

    declareMethod(jni_gamecenter_api::openAchievement, "openAchievement", "()V");
    declareMethod(jni_gamecenter_api::openLeaderboards, "openLeaderboards", "()V");
    declareMethod(jni_gamecenter_api::openSpecificLeaderboard, "openSpecificLeaderboard", "(I)V");
    declareMethod(jni_gamecenter_api::openDashboard, "openDashboard", "()V");
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
void GameCenterAPIAndroidImpl::disconnect() {
        env->CallVoidMethod(instance, methods[jni_gamecenter_api::disconnect]);
}


void GameCenterAPIAndroidImpl::unlockAchievement(int id) {
        env->CallVoidMethod(instance, methods[jni_gamecenter_api::unlockAchievement], id);
}
void GameCenterAPIAndroidImpl::updateAchievementProgression(int id, int stepReached) {
        env->CallVoidMethod(instance, methods[jni_gamecenter_api::updateAchievementProgression], id, stepReached);
}

void GameCenterAPIAndroidImpl::submitScore(int leaderboardID, const std::string & score) {
    jstring jscore = env->NewStringUTF(score.c_str());
        env->CallVoidMethod(instance, methods[jni_gamecenter_api::submitScore], leaderboardID, jscore);
}

void GameCenterAPIAndroidImpl::getWeeklyRank(int leaderboardID, std::function<void (int rank)> func) {
    weeklyRankFunctions.push_back(func);
    env->CallVoidMethod(instance, methods[jni_gamecenter_api::getWeeklyRank], leaderboardID);
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
        env->CallVoidMethod(instance, methods[jni_gamecenter_api::openDashboard]);
}
