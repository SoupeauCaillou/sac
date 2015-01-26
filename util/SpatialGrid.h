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

#include <stdint.h>
#include <functional>
#include <glm/glm.hpp>
#include "base/Entity.h"

#include <list>
#include <map>
#include <vector>
#include <ostream>
#include <util/IntersectionUtil.h>

// let's start with a dumb position data structure
class GridPos {
    friend class SpatialGrid;

    public:
    GridPos(int32_t q = 0, int32_t r = 0);

    bool operator<(const GridPos& p) const;
    bool operator==(const GridPos& p) const;
    bool operator!=(const GridPos& p) const;
    GridPos operator-(const GridPos& p) const;
    GridPos operator+(const GridPos& p) const;
    int32_t q, r;

    friend std::ostream& operator<<(std::ostream& str, const GridPos& gp);
};

struct Cell {
    std::list<Entity> entities;
};

class SpatialGrid {
    public:
    struct Iterate {
        enum Options { LeftToRightTopToBottom };
        struct Result {
            GridPos pos;
            bool valid, newLine;
        };
    };

    protected:
    virtual ~SpatialGrid(){};
    SpatialGrid(){};

    public:
    virtual bool isPosValid(const GridPos& pos) const = 0;

    virtual bool isPathBlockedAt(const GridPos& npos, Entity* by = 0) const = 0;
    virtual bool isVisibilityBlockedAt(const GridPos& npos) const = 0;

    virtual std::vector<GridPos>
    getNeighbors(const GridPos& pos, bool enableInvalidPos = false) const = 0;
    virtual GridPos positionToGridPos(const glm::vec2& pos) const = 0;
    virtual glm::vec2 gridPosToPosition(const GridPos& gp) const = 0;
    virtual void forEachCellDo(std::function<void(const GridPos&)> f) = 0;
    virtual void addEntityAt(Entity e,
                             const GridPos& p,
                             bool updateSpatialPosition = false) = 0;
    virtual void removeEntityFrom(Entity e, const GridPos& p) = 0;
    virtual std::list<Entity>& getEntitiesAt(const GridPos& p) = 0;
    virtual void
    autoAssignEntitiesToCell(const std::vector<Entity>& entities) = 0;
    virtual Iterate::Result
    iterate(GridPos pos,
            Iterate::Options opt = Iterate::LeftToRightTopToBottom) const = 0;

    virtual int gridPosMoveCost(const GridPos& from,
                                const GridPos& to) const = 0;
    virtual std::map<int, std::vector<GridPos>>
    movementRange(const GridPos& p, int movement) const = 0;
    virtual std::vector<GridPos> viewRange(const GridPos& p,
                                           int size) const = 0;
    virtual std::vector<GridPos>
    ringFinder(const GridPos& p, int range, bool enableInvalidPos) const = 0;
    virtual std::vector<GridPos> lineDrawer(const GridPos& from,
                                            const GridPos& to,
                                            bool positiveEps = true) const = 0;
    virtual int canDrawLine(const GridPos& p1, const GridPos& p2) const = 0;

    virtual std::vector<GridPos>
    findPath(const GridPos& from,
             const GridPos& to,
             bool ignoreBlockedEndPath = false) const = 0;

    virtual unsigned computeGridDistance(const glm::vec2& p1,
                                         const glm::vec2& p2) const = 0;
    virtual unsigned computeGridDistance(const GridPos& p1,
                                         const GridPos& p2) const = 0;

    virtual float computeRealDistance(const glm::vec2& p1,
                                      const glm::vec2& p2) const = 0;
    virtual float computeRealDistance(const GridPos& p1,
                                      const GridPos& p2) const = 0;

    virtual AABB boundingBox(bool inner) const = 0;

    protected:
    std::map<GridPos, Cell> cells;
    std::map<Entity, std::list<GridPos>>
        entityToGridPos; // only for single place
};

#endif
