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
#include "base/EntityManager.h"

#include <vector>
#include <fstream>

class GameCenterAPIDebugImpl : public GameCenterAPI {
    public:

        void connectOrRegister();
        void disconnect();

        bool isConnected();
        bool isRegistered();

        void unlockAchievement(int id);
        void updateAchievementProgression(int id, int stepReached);

        void submitScore(int leaderboardID, const std::string & score);

        void openAchievement();
        void openLeaderboards();
        void openSpecificLeaderboard(int id);
        void openDashboard();
    
    private:
        bool _isConnected = false;

        std::stringstream message;
        Entity createAutodestroySuccess(float duration);
        void  displayAction(float duration = 2.5f);
};
