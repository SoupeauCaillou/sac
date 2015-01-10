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

#ifdef __cplusplus
extern "C" {
#endif

struct Game;

struct SetupInfo {
    const char* name;
    const char* version;
    struct { float x, y; } resolution;
};

struct Game* buildGameInstance();

// SETUP WINDOW, GL, stuff
int setupEngine(struct Game* game, const struct SetupInfo* info);
int tickEngine(struct Game* game);

int initGame(const char* title, const char* version);
// START GAME
int launchGame(struct Game* gameImpl, int argc, char** argv);

#ifdef __cplusplus
}
#endif