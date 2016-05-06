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
#include "BackInTimeSystem.h"

#if SAC_DEBUG
#include "util/Draw.h"
#endif


typedef glm::ivec2 CellCoords;
typedef std::vector<Entity> Cell; //...

std::vector<Cell> cells;
std::vector<glm::ivec2> coords;

INSTANCE_IMPL(SpatialPartitionSystem);

SpatialPartitionSystem::SpatialPartitionSystem() : ComponentSystemImpl<SpatialPartitionComponent>(HASH("SpatialPartition", 0x35df9814)) {
    SpatialPartitionComponent tc;
    componentSerializer.add(new Property<int>(HASH("count", 0x78b8273a), OFFSET(count, tc)));
    cellSize = 3;

    #if SAC_DEBUG
    showDebug = false;
    #endif
}

#if SAC_DEBUG
void drawDebug(float cellSize, const glm::vec2& minPos, int total, glm::ivec2 gridSize) {
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
        const auto* hc = BACK_IN_TIME(e);
        float maxSizeComp =
            glm::max(tc->size.x, glm::max(tc->size.y, glm::max(hc->size.x, hc->size.y)));
        minPos = glm::min(minPos, glm::min(tc->position, hc->position) - maxSizeComp);
        maxPos = glm::max(maxPos, glm::max(tc->position, hc->position) + maxSizeComp);
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
    int count = 0;

    FOR_EACH_ENTITY_COMPONENT(SpatialPartition, e, comp)
        const auto* tc = TRANSFORM(e);
        AABB aabb;
        {
            const auto* hc = BACK_IN_TIME(e);
            AABB frames[2];
            IntersectionUtil::computeAABB(
                tc->position - minPos,
                tc->size,
                tc->rotation,
                frames[0]);
            IntersectionUtil::computeAABB(
                hc->position - minPos,
                hc->size,
                hc->rotation,
                frames[1]);
            aabb = IntersectionUtil::mergeAABB(frames, 2);
        }

        // insert entity in all cells covered by AABB
        CellCoords cellTopLeft = positionToCellCoords(glm::vec2(aabb.left, aabb.top), invCellSize);
        CellCoords cellBottomRight = positionToCellCoords(glm::vec2(aabb.right, aabb.bottom), invCellSize);

        int cellCount = (cellTopLeft.y - cellBottomRight.y + 1) *
            (cellBottomRight.x - cellTopLeft.x + 1);
        coords.resize(count + cellCount);
        comp->cellOffset = count;
        comp->count = 0;
        for (int y=cellBottomRight.y; y<=cellTopLeft.y; y++) {
            for (int x=cellTopLeft.x; x<=cellBottomRight.x; x++) {
                cells[y * gridSize.x + x].push_back(e);
                coords[count] = glm::ivec2(x, y);
                count++;
                comp->count++;
            }
        }
    }

    #if SAC_DEBUG
    if (showDebug) {
        drawDebug(cellSize, minPos, entityWithComponent.size(), gridSize);
    }
    #endif
}

glm::ivec2* SpatialPartitionSystem::getCells(int offset) {
    LOGF_IF(offset >= coords.size(), "Invalid cell offset " << __(offset) << ". Size is " << __(coords.size()));
    return &coords[offset];
}
