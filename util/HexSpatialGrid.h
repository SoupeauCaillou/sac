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

#if !DISABLE_GRID_SYSTEM

#include "SpatialGrid.h"
#include <stdint.h>
#include <functional>
#include <glm/glm.hpp>
#include "base/Entity.h"

#include <list>
#include <map>
#include <vector>
#include <ostream>

/**
 * Hexagonal grid object which can be used for any grid-based game.
 *
 * @details This object is completely space independent: you should be able
 * to retrieve neighbors of a cell simply with cell[i+1] for instance.
 * Space position MUST be handled with #GridPos object.
 */
class HexSpatialGrid : public SpatialGrid {

    public:
    /**
     * @param width number of cells on X-axis. Must be odd.
     * @param height number of cells on Y-axis. Must be odd.
     * @param hexagonWidth absolute space width of a cell (NB: this is the only
     * parameter related to space problematic).
     * See TransformationSystem for space management.
     */
    HexSpatialGrid(int width, int height, float hexagonWidth);
    ~HexSpatialGrid();

    virtual bool isPosValid(const GridPos& pos) const;

    virtual bool isPathBlockedAt(const GridPos& npos, Entity* by = 0) const;
    virtual bool isVisibilityBlockedAt(const GridPos& npos) const;

    virtual std::vector<GridPos>
    getNeighbors(const GridPos& pos, bool enableInvalidPos = false) const;
    virtual GridPos positionToGridPos(const glm::vec2& pos) const;
    virtual glm::vec2 gridPosToPosition(const GridPos& gp) const;
    virtual void forEachCellDo(std::function<void(const GridPos&)> f);
    virtual void
    addEntityAt(Entity e, const GridPos& p, bool updateSpatialPosition = false);
    virtual void removeEntityFrom(Entity e, const GridPos& p);
    virtual std::list<Entity>& getEntitiesAt(const GridPos& p);
    virtual void autoAssignEntitiesToCell(const std::vector<Entity>& entities);
    virtual Iterate::Result
    iterate(GridPos pos,
            Iterate::Options opt = Iterate::LeftToRightTopToBottom) const;

    virtual int gridPosMoveCost(const GridPos& from, const GridPos& to) const;
    virtual std::map<int, std::vector<GridPos>>
    movementRange(const GridPos& p, int movement) const;
    virtual std::vector<GridPos> viewRange(const GridPos& p, int size) const;
    virtual std::vector<GridPos>
    ringFinder(const GridPos& p, int range, bool enableInvalidPos) const;
    virtual std::vector<GridPos> lineDrawer(const GridPos& from,
                                            const GridPos& to,
                                            bool positiveEps = true) const;
    virtual int canDrawLine(const GridPos& p1, const GridPos& p2) const;

    virtual std::vector<GridPos>
    findPath(const GridPos& from,
             const GridPos& to,
             bool ignoreBlockedEndPath = false) const;

    virtual unsigned computeGridDistance(const glm::vec2& p1,
                                         const glm::vec2& p2) const;
    virtual unsigned computeGridDistance(const GridPos& p1,
                                         const GridPos& p2) const;

    virtual float computeRealDistance(const glm::vec2& p1,
                                      const glm::vec2& p2) const;
    virtual float computeRealDistance(const GridPos& p1,
                                      const GridPos& p2) const;

    virtual AABB boundingBox(bool inner) const;

    int getWidth() const { return w; }
    int getHeight() const { return h; }

    private:
    GridPos cubeCoordinateRounding(float x, float y, float z) const;
    GridPos positionSizeToGridPos(const glm::vec2& pos /*, float size*/) const;

    private:
    int w, h;
    float size;
};

#endif
