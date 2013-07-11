#include "GridSystem.h"

INSTANCE_IMPL(GridSystem);

GridSystem::GridSystem() : ComponentSystemImpl<GridComponent>("Grid") {
    GridComponent tc;
    componentSerializer.add(new Property<bool>("type", OFFSET(type, tc)));
}

void GridSystem::DoUpdate(float) {
}

