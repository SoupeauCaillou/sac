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

#include <string>
#include <functional>

class GameCenterAPI {
	public:

        virtual void connectOrRegister() = 0;
        virtual void disconnect() = 0;

        virtual bool isRegistered() = 0;
        virtual bool isConnected() = 0;

        virtual void unlockAchievement(int id) = 0;
        virtual void updateAchievementProgression(int id, int stepReached) = 0;

        virtual void submitScore(int leaderboardID, const std::string & score) = 0;
        virtual void getWeeklyRank(int leaderboardID, std::function<void (int rank)> func) = 0;

        virtual void openAchievement() = 0;
        virtual void openLeaderboards() = 0;
        virtual void openSpecificLeaderboard(int id) = 0;
        virtual void openDashboard() = 0;
};
