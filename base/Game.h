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

#include <stdint.h>
#include <string>
#include <vector>
#include "GameContext.h"

class MouseNativeTouchState;

#if SAC_INGAME_EDITORS

#include "../util/LevelEditor.h"

namespace GameType {
    enum Enum {
        Default,
        LevelEditor,
        SingleStep
    };
}
#endif

class ComponentSystem;

class Game {
	public:
		Game();
		virtual ~Game();

        void setGameContexts(GameContext* gameThreadContext, GameContext* renderThreadContext);
        void step();
        void render();
        void resetTime();
        void eventsHandler();

        virtual void backPressed() {};
        virtual bool willConsumeBackEvent() { return false; }

        virtual bool wantsAPI(ContextAPI::Enum api) const = 0;
		virtual void sacInit(int windowW, int windowH);
		virtual void init(const uint8_t* in = 0, int size = 0) = 0;
        virtual void quickInit() = 0;
		virtual int saveState(uint8_t** out);
        virtual void togglePause(bool) { }
	protected:
		void loadFont(AssetAPI* asset, const std::string& name);
    private:
        virtual void tick(float dt) = 0;

    public:
        GameContext* gameThreadContext, *renderThreadContext;

        //tough... only needed for mouse events handling
        MouseNativeTouchState * mouseNativeTouchState;

        float targetDT;

        bool isFinished;

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
#if SAC_INGAME_EDITORS
        GameType::Enum gameType;
        LevelEditor* levelEditor;
#endif
    public:
        void buildOrderedSystemsToUpdateList();
    private:
        std::vector<ComponentSystem*> orderedSystemsToUpdate;
};
