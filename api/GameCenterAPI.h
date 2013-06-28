#pragma once

#include <string>

class GameCenterAPI {
	public:

        virtual void connectOrRegister() = 0;

        virtual bool isRegistered() = 0;
        virtual bool isConnected() = 0;

        virtual void unlockAchievement(int id) = 0;

        virtual void openAchievement() = 0;
        virtual void openLeaderboards() = 0;
        virtual void openSpecificLeaderboard(int id) = 0;
        virtual void openDashboard() = 0;
};
