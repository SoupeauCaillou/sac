#include "GridSystem.h"

INSTANCE_IMPL(GridSystem);

GridSystem::GridSystem() : ComponentSystemImpl<GridComponent>("Grid") {
    GridComponent tc;
    componentSerializer.add(new Property<bool>("blocks_vision", OFFSET(blocksVision, tc)));
    componentSerializer.add(new Property<bool>("blocks_path", OFFSET(blocksPath, tc)));
    componentSerializer.add(new Property<bool>("can_be_on_multiple_cells", OFFSET(canBeOnMultipleCells, tc)));

}

void GridSystem::DoUpdate(float) {
}

