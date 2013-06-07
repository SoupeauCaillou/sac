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

        GridPos positionToGridPos(const glm::vec2& pos) const;
        glm::vec2 gridPosToPosition(const GridPos& gp) const;

        void doForEachCell(std::function<void(const GridPos& )> f);

        void addEntityAt(Entity e, const GridPos& p);
        void removeEntityFrom(Entity e, const GridPos& p);

        std::list<Entity>& getEntitiesAt(const GridPos& p);

        void autoAssignEntitiesToCell(std::list<Entity> entities);

        std::map<int, std::vector<GridPos> > movementRange(GridPos& p, int movement);
        std::vector<GridPos> viewRange(GridPos& p, int size);
        std::vector<GridPos> ringFinder(GridPos& p, int range, bool enableInvalidPos);
        std::vector<GridPos> lineDrawer(GridPos& p1, GridPos& p2);

        std::vector<GridPos> findPath(const GridPos& from, const GridPos& to) const;

	public:
		static unsigned ComputeDistance(const GridPos& p1, const GridPos& p2);

	private:
		struct SpatialGridData;
		SpatialGridData* datas;

};
