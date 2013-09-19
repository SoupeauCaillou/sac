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
