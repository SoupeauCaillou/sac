/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <stdint.h>
#include <string>
class AssetAPI;

#ifdef INGAME_EDITORS

#include "../util/LevelEditor.h"

namespace GameType {
    enum Enum {
        Default,
        LevelEditor
    };
}
#endif

class Game {
	public:
		Game();

		virtual ~Game();
		
		virtual void sacInit(int windowW, int windowH);
		
		virtual void init(const uint8_t* in = 0, int size = 0) = 0;
		
        void step();

        void render();

		virtual void backPressed();
		
		virtual int saveState(uint8_t** out);

        virtual bool willConsumeBackEvent() { return false; }
        
        virtual void togglePause(bool pause) { }

        void resetTime();
		
        float targetDT;

	protected:
		void loadFont(AssetAPI* asset, const std::string& name);
    private:
        virtual void tick(float dt) = 0;

    struct {
        float minDt, maxDt;
        float since;
        int frameCount;
        void reset(float timeMark) {
            minDt = 10000000;
            maxDt = 0;
            since = timeMark;
            frameCount = 0;
        }
    } fpsStats;
    float lastUpdateTime;
#ifdef INGAME_EDITORS
    GameType::Enum gameType;
    LevelEditor levelEditor;
#endif
};