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
#include "util/SerializerProperty.h"

INSTANCE_IMPL(GridSystem);

GridSystem::GridSystem() : ComponentSystemImpl<GridComponent>(HASH("Grid", 0xb87b426b)) {
    GridComponent tc;
    componentSerializer.add(new Property<int>(HASH("type", 0xf3ebd1bf), OFFSET(type, tc)));
    componentSerializer.add(new Property<bool>(HASH("blocks_path", 0x601b5a16), OFFSET(blocksPath, tc)));
    componentSerializer.add(new Property<bool>(HASH("blocks_vision", 0x3dc1d8a2), OFFSET(blocksVision, tc)));
    componentSerializer.add(new Property<bool>(HASH("can_be_on_multiple_cells", 0x840cdec3), OFFSET(canBeOnMultipleCells, tc)));
}

void GridSystem::DoUpdate(float) {
}
#endif
