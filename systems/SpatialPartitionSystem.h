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

struct TransformationComponent;

namespace spatial_partition_update_mode
{
    enum Enum { Automatic = 0, Manual };
}

struct SpatialPartitionComponent {
    SpatialPartitionComponent() :
        mode(spatial_partition_update_mode::Automatic),
        partitionId(0) {}
    spatial_partition_update_mode::Enum mode;

    uint32_t partitionId;
};

#define theSpatialPartitionSystem SpatialPartitionSystem::GetInstance()
#if SAC_DEBUG
#define SPATIAL_PARTITION(e) theSpatialPartitionSystem.Get(e, true, __FILE__, __LINE__)
#else
#define SPATIAL_PARTITION(e) theSpatialPartitionSystem.Get(e)
#endif

UPDATABLE_SYSTEM(SpatialPartition)

#if SAC_DEBUG
    public:
        bool showDebug;
#endif

};
