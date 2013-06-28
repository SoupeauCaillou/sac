#pragma once

#include "api/GameCenterAPI.h"

#include "base/Log.h"

#include <vector>
#include <fstream>

class GameCenterAPIDebugImpl : public GameCenterAPI {
    private:
        bool _isConnected = false;
        std::vector<std::string> achievements;
	public:
        GameCenterAPIDebugImpl() {
            std::ifstream iff("successes_name.txt");
            int indice = 0;
            while (iff.good()) {
                getline(iff, achievements[indice++]);
            }
        }
        ~GameCenterAPIDebugImpl() {}

        void connectOrRegister() { LOGI("connecting."); _isConnected = true; }

        bool isConnected() { return _isConnected; }
        bool isRegistered() { return true; }

        void unlockAchievement(int id) { LOGI("Unlocked success " << id << ": " << achievements[id-1]); }

        void openAchievement() { LOGI("openAchievement."); }
        void openLeaderboards() { LOGI("openLeaderboards."); }
        void openSpecificLeaderboard(int id) { LOGI("openSpecificLeaderboard." << id); }
        void openDashboard() { LOGI("openDashboard."); }
};
