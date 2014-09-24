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

#include <glm/glm.hpp>

class Game;

class LevelEditor {
    public:
        LevelEditor(Game* game);
        ~LevelEditor();
        void init();
        void tick(float dt);

        static void lock();
        static void unlock();
    private:
        struct LevelEditorDatas;
        LevelEditorDatas* datas;
        Game* game;

#if SAC_INGAME_EDITORS
    public:
        static glm::vec2 GameViewPosition();
        static int DebugAreaWidth;
        static int DebugAreaHeight;
#endif
};
