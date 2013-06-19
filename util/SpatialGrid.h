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



class SpatialGrid {
	public:
		SpatialGrid(int w, int h, float hexagonWidth = 1);

	public:
		std::vector<GridPos> getNeighbors(const GridPos& pos, bool enableInvalidPos) const;
        bool isPathBlockedAt(const GridPos& npos) const;

        GridPos positionToGridPos(const glm::vec2& pos) const;
        glm::vec2 gridPosToPosition(const GridPos& gp) const;

        void doForEachCell(std::function<void(const GridPos& )> f);

        void addEntityAt(Entity e, const GridPos& p);
        void removeEntityFrom(Entity e, const GridPos& p);

        std::list<Entity>& getEntitiesAt(const GridPos& p);

        void autoAssignEntitiesToCell(const std::vector<Entity>& entities);

        std::map<int, std::vector<GridPos> > movementRange(const GridPos& p, int movement) const;
        std::vector<GridPos> viewRange(const GridPos& p, int size) const;
        std::vector<GridPos> ringFinder(const GridPos& p, int range, bool enableInvalidPos) const;
        std::vector<GridPos> lineDrawer(const GridPos& p1, const GridPos& p2) const;
        int canDrawLine(const GridPos& p1, const GridPos& p2) const;

        std::vector<GridPos> findPath(const GridPos& from, const GridPos& to, bool ignoreBlockedEndPath = false) const;

        unsigned computeGridDistance(const glm::vec2& p1, const glm::vec2& p2) const;

	public:
		static unsigned ComputeDistance(const GridPos& p1, const GridPos& p2);

	private:
		struct SpatialGridData;
		SpatialGridData* datas;

};
