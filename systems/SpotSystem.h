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

#include "System.h"
#include "opengl/Polygon.h"

struct SpotComponent {
	SpotComponent(): angle(6.28318530718), distance(10), resolution(36) {}

	float angle;
	float distance;
    int resolution;

    Polygon area;
};

#define theSpotSystem SpotSystem::GetInstance()
#if SAC_DEBUG
#define SPOT(e) theSpotSystem.Get(e,true,__FILE__,__LINE__)
#else
#define SPOT(e) theSpotSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Spot)
};


struct SpotBlockComponent {
};

#define theSpotBlockSystem SpotBlockSystem::GetInstance()
#if SAC_DEBUG
#define SPOT_BLOCK(e) theSpotBlockSystem.Get(e,true,__FILE__,__LINE__)
#else
#define SPOT_BLOCK(e) theSpotBlockSystem.Get(e)
#endif

UPDATABLE_SYSTEM(SpotBlock)
};
