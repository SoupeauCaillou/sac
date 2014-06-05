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

#if !DISABLE_GRID_SYSTEM

#include "GridSystem.h"

INSTANCE_IMPL(GridSystem);

GridSystem::GridSystem() : ComponentSystemImpl<GridComponent>(HASH("Grid", 0x0)) {
    GridComponent tc;
    componentSerializer.add(new Property<int>(HASH("type", 0xf3ebd1bf), OFFSET(type, tc)));
    componentSerializer.add(new Property<bool>(HASH("blocks_path", 0x601b5a16), OFFSET(blocksPath, tc)));
    componentSerializer.add(new Property<bool>(HASH("blocks_vision", 0x3dc1d8a2), OFFSET(blocksVision, tc)));
    componentSerializer.add(new Property<bool>(HASH("can_be_on_multiple_cells", 0x840cdec3), OFFSET(canBeOnMultipleCells, tc)));
}

int GridSystem::GetVisibilityCost(GridComponent::EType type, int distance) {
    switch (type) {
        case GridComponent::Pit:
        case GridComponent::Normal:
            return 1;
        case GridComponent::Brush:
            return 1 + (glm::min(distance + 1, 6) / 2 );
        case GridComponent::House:
            return 1;
    }
    LOGF("Shouldn't be here!");
    return 0;
}

int GridSystem::GetAttackBonus(GridComponent::EType type) {
    switch (type) {
        case GridComponent::Brush:
            return 1;
        case GridComponent::House:
            return 2;
        default:
            return 0;
    }
}

int GridSystem::GetDefenceBonus(GridComponent::EType type) {
    switch (type) {
        case GridComponent::Pit:
            return 1;
        case GridComponent::Brush:
            return 1;
        case GridComponent::House:
            return 2;
        default:
            return 0;
    }
}

void GridSystem::DoUpdate(float) {
}
#endif
