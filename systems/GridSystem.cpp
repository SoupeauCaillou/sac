#include "GridSystem.h"

INSTANCE_IMPL(GridSystem);

GridSystem::GridSystem() : ComponentSystemImpl<GridComponent>("Grid") {
    MaximumAttackBonus = GetAttackBonus(GridComponent::House);

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
            return 3;
    }
    LOGF("Shouldn't be here!");
    return 0;
}

int GridSystem::GetAttackBonus(GridComponent::EType type) {
    switch (type) {
        case GridComponent::Pit:
        case GridComponent::Normal:
            return 1;
        case GridComponent::Brush:
            return 1;
        case GridComponent::House:
            return 3;
    }
    LOGF("Shouldn't be here!");
    return 0;
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

