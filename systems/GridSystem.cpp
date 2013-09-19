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



#include "GridSystem.h"

INSTANCE_IMPL(GridSystem);

GridSystem::GridSystem() : ComponentSystemImpl<GridComponent>("Grid") {
    GridComponent tc;
    componentSerializer.add(new Property<int>("type", OFFSET(type, tc)));
    componentSerializer.add(new Property<bool>("blocks_path", OFFSET(blocksPath, tc)));
    componentSerializer.add(new Property<bool>("blocks_vision", OFFSET(blocksVision, tc)));
    componentSerializer.add(new Property<bool>("can_be_on_multiple_cells", OFFSET(canBeOnMultipleCells, tc)));
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

