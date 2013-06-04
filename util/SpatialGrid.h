#pragma once

#include <stdint.h>
#include <vector>
#include <glm/glm.hpp>

// let's start with a dumb position data structure
class GridPos {
	friend class SpatialGrid;
public:
	GridPos(int32_t q = 0, int32_t r = 0);

	int32_t q, r;
};

class SpatialGrid {
	public:
		SpatialGrid(int w, int h, float hexagonWidth = 1);

	public:
		std::vector<GridPos> getNeighbors(const GridPos& pos) const;

        GridPos positionToGridPos(const glm::vec2& pos) const;
        glm::vec2 gridPosToPosition(const GridPos& gp) const;

	public:
		static unsigned ComputeDistance(const GridPos& p1, const GridPos& p2);

	private:
		struct SpatialGridData;
		SpatialGridData* datas;

};
