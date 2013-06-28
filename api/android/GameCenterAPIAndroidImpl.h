#pragma once

#include "api/GameCenterAPI.h"
#include "JNIWrapper.h"

namespace jni_gamecenter_api {
    enum Enum {
        connectOrRegister,
        isRegistered,
        isConnected,
        unlockAchievement,
        openAchievement,
        openLeaderboards,
        openSpecificLeaderboard,
        openDashboard,
    };
}
class GameCenterAPIAndroidImpl : public GameCenterAPI, public JNIWrapper<jni_gamecenter_api::Enum> {
    public:
        GameCenterAPIAndroidImpl();

        void init(JNIEnv* env);

        void connectOrRegister();

        bool isRegistered();
        bool isConnected();

        void unlockAchievement(int id);

        void openAchievement();
        void openLeaderboards();
        void openSpecificLeaderboard(int id);
        void openDashboard();
    public:
        JNIEnv* env;
};

