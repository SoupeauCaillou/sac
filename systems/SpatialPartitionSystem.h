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

struct SpatialPartitionComponent {
    SpatialPartitionComponent() :
        cells(0), count(0) {}

    glm::ivec2* cells;
    int count;
};

#define theSpatialPartitionSystem SpatialPartitionSystem::GetInstance()
#if SAC_DEBUG
#define SPATIAL_PARTITION(e) theSpatialPartitionSystem.Get(e, true, __FILE__, __LINE__)
#else
#define SPATIAL_PARTITION(e) theSpatialPartitionSystem.Get(e)
#endif

UPDATABLE_SYSTEM(SpatialPartition)

    public:
        float cellSize;
#if SAC_DEBUG
        bool showDebug;
#endif

};
