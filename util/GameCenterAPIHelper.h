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

#include "base/Entity.h"

#include "api/GameCenterAPI.h"

#include <string>

#include "FaderHelper.h"

class GameCenterAPIHelper {
	public:
		void init(GameCenterAPI * gameCenterAPI, bool useAchievements, 
            bool useLeaderboards, bool uniqueLeaderboard);
	    
	    void displayUI();
		void hideUI();
        //do not forget to call it if you display UI otherwise inputs won't be handled!
        //return true if a button has been clicked
        bool updateUI(); 

        void registerForFading(FaderHelper* fader, Fading::Enum type);

	private:
        void displayFeatures(bool display);

        bool uniqueLeaderboard;
		GameCenterAPI * gameCenterAPI;
        Entity signButton, achievementsButton, leaderboardsButton;
        bool bUIdisplayed;
};
