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

#if !DISABLE_DEBUGGING_SYSTEM || SAC_DEBUG

#pragma once

#include "System.h"
#include <vector>

struct DebuggingComponent {};

#define theDebuggingSystem DebuggingSystem::GetInstance()
#if SAC_DEBUG
#define DEBUGGING(e) theDebuggingSystem.Get(e, true, __FILE__, __LINE__)
#else
#define DEBUGGING(e) theDebuggingSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Debugging)

public:
void toggle();

private:
bool enable;
std::map<hash_t, Entity> debugEntities;
std::vector<Entity> renderStatsEntities;

Entity fps, entityCount, systems;
Entity fpsLabel, entityCountLabel;
}
;

#endif
