#pragma once

#include <stdint.h>
#include <functional>
#include <glm/glm.hpp>
#include "base/Entity.h"

#include <list>
#include <map>
#include <vector>
#include <ostream>

// let's start with a dumb position data structure
class GridPos {
	friend class SpatialGrid;
public:
	GridPos(int32_t q = 0, int32_t r = 0);

    bool operator<(const GridPos& p) const;
	bool operator==(const GridPos& p) const;
	bool operator!=(const GridPos& p) const;
	int32_t q, r;

    friend std::ostream& operator<<(std::ostream& str, const GridPos& gp);
};

struct Cell {
    std::list<Entity> entities;
};

struct SpatialGridData {
    int w, h;
    float size;
    std::map<GridPos, Cell> cells;
    std::map<Entity, std::list<GridPos>> entityToGridPos; // only for single place

    SpatialGridData(int pW, int pH, float hexagonWidth);

    bool isPosValid(const GridPos& pos) const ;

    bool isPathBlockedAt(const GridPos& npos) const;
    bool isVisibilityBlockedAt(const GridPos& npos) const;
};

class SpatialGrid {
	public:
		SpatialGrid(int w, int h, float hexagonWidth = 1);
        virtual ~SpatialGrid();

	public:
		std::vector<GridPos> getNeighbors(const GridPos& pos, bool enableInvalidPos = false) const;
        GridPos positionToGridPos(const glm::vec2& pos) const;
        glm::vec2 gridPosToPosition(const GridPos& gp) const;
        void forEachCellDo(std::function<void(const GridPos& )> f);
        void addEntityAt(Entity e, const GridPos& p);
        void removeEntityFrom(Entity e, const GridPos& p);
        std::list<Entity>& getEntitiesAt(const GridPos& p) const;
        void autoAssignEntitiesToCell(const std::vector<Entity>& entities);
        unsigned computeGridDistance(const glm::vec2& p1, const glm::vec2& p2) const;

        virtual bool isPathBlockedAt(const GridPos& npos) const;
        virtual int gridPosMoveCost(const GridPos& from, const GridPos& to) const;
        virtual std::map<int, std::vector<GridPos> > movementRange(const GridPos& p, int movement) const;
        virtual std::vector<GridPos> viewRange(const GridPos& p, int size) const;
        virtual std::vector<GridPos> ringFinder(const GridPos& p, int range, bool enableInvalidPos) const;
        virtual std::vector<GridPos> lineDrawer(const GridPos& from, const GridPos& to) const;
        virtual int canDrawLine(const GridPos& p1, const GridPos& p2) const;

        virtual std::vector<GridPos> findPath(const GridPos& from, const GridPos& to, bool ignoreBlockedEndPath = false) const;


	public:
		static unsigned ComputeDistance(const GridPos& p1, const GridPos& p2);

	public: // arg
		SpatialGridData* datas;

};
