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

#include "SpatialPartitionSystem.h"
#include "util/SerializerProperty.h"
#include "util/IntersectionUtil.h"
#include "TransformationSystem.h"

#if SAC_DEBUG
#include "util/Draw.h"
#endif


typedef glm::ivec2 CellCoords;
typedef std::vector<Entity> Cell; //...

std::vector<Cell> cells;
glm::ivec2 gridSize;
std::vector<glm::ivec2> coords;

INSTANCE_IMPL(SpatialPartitionSystem);

SpatialPartitionSystem::SpatialPartitionSystem() : ComponentSystemImpl<SpatialPartitionComponent>(HASH("SpatialPartition", 0x35df9814)) {
    SpatialPartitionComponent tc;
    componentSerializer.add(new Property<int>(HASH("count", 0), OFFSET(count, tc)));
    cellSize = 3;

    #if SAC_DEBUG
    showDebug = false;
    #endif
}

#if SAC_DEBUG
void drawDebug(float cellSize, const glm::vec2& minPos, int total) {
    float avg = total / (float)cells.size();
    for (size_t i=0; i<cells.size(); i++) {
        if (cells[i].empty()) {
            continue;
        }
        int y = i / gridSize.x;
        int x = i % gridSize.x;

        Draw::Rectangle(
            minPos + glm::vec2(cellSize * (x + 0.5f), cellSize * (y + 0.5f)),
            glm::vec2(cellSize, cellSize), 0.0f,
            Color::palette(cells[i].size() / avg, 0.8));
    }
}
#endif


CellCoords positionToCellCoords(const glm::vec2& position, float invCellSize) {
    CellCoords result = CellCoords((int) glm::floor(position.x * invCellSize), (int) glm::floor(position.y * invCellSize));
    return result;
}

void SpatialPartitionSystem::DoUpdate(float) {
    const float invCellSize = 1.0f / cellSize;
    glm::vec2 minPos(FLT_MAX, FLT_MAX), maxPos(-FLT_MAX, -FLT_MAX);

    FOR_EACH_ENTITY_COMPONENT(SpatialPartition, e, comp)
        const auto* tc = TRANSFORM(e);
        glm::vec2 size(glm::max(tc->size.x, tc->size.y));
        minPos = glm::min(minPos, tc->position - size);
        maxPos = glm::max(maxPos, tc->position + size);
    }

    // make sure cell storage is correctly sized
    // and reset storage
    CellCoords ma = positionToCellCoords(maxPos, invCellSize);
    CellCoords mi = positionToCellCoords(minPos, invCellSize);

    gridSize.x = (ma.x - mi.x + 1);
    gridSize.y = (ma.y - mi.y + 1);
    int cellCount = gridSize.x *gridSize.y;
    cells.resize(cellCount);
    for (int i=0; i<cellCount; i++) {
        cells[i].clear();
    }
    coords.clear();

    FOR_EACH_ENTITY_COMPONENT(SpatialPartition, e, comp)
        AABB aabb;
        const auto* tc = TRANSFORM(e);
        IntersectionUtil::computeAABB(
            tc->position - minPos,
            tc->size,
            tc->rotation,
            aabb);

        // insert entity in all cells covered by AABB
        CellCoords cellTopLeft = positionToCellCoords(glm::vec2(aabb.left, aabb.top), invCellSize);
        CellCoords cellBottomRight = positionToCellCoords(glm::vec2(aabb.right, aabb.bottom), invCellSize);

        comp->cells = &coords[coords.size()];
        comp->count = 0;
        for (int y=cellBottomRight.y; y<=cellTopLeft.y; y++) {
            for (int x=cellTopLeft.x; x<=cellBottomRight.x; x++) {
                cells[y * gridSize.x + x].push_back(e);
                coords.push_back(glm::ivec2(x, y));
                comp->count++;
            }
        }
    }

    #if SAC_DEBUG
    if (showDebug) {
        drawDebug(cellSize, minPos, entityWithComponent.size());
    }
    #endif
}
