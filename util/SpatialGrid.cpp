#include "SpatialGrid.h"
#include "base/Log.h"
#include <map>


static uint64_t Hash(const GridPos& p) {
	uint64_t h = p.q;
	h <<= 32;
	return h | p.r;
}

struct Cell {

};

GridPos::GridPos(int32_t pQ, int32_t pR) : q(pQ), r(pR) {

}

struct SpatialGrid::SpatialGridData {
	int w, h;
	std::map<uint64_t, Cell> cells;

	SpatialGridData(int pW, int pH) : w(pW), h(pH) {
		LOGF_IF(w % 2 == 0, "Please use even sized grid. Invalid width: " << w);
		LOGF_IF(h % 2 == 0, "Please use even sized grid. Invalid height: " << h);
		for (int i=-w/2; i<=w/2; i++) {
			for (int j=-h/2; j<=h/2; j++) {
				cells.insert(std::make_pair(Hash(GridPos(i, j)), Cell()));
			}
		}
	}

	bool isPosValid(const GridPos& pos) const {
		return glm::abs(pos.q) <= (int)(w/2) &&
			glm::abs(pos.r) <= (int)(h/2);
	}

};

SpatialGrid::SpatialGrid(int w, int h) {
	datas = new SpatialGridData(w, h);
}


std::vector<GridPos> SpatialGrid::getNeighbors(const GridPos& pos) const {
	std::vector<GridPos> n;
	int offsets[] = {
		1, 0,
		1, -1,
		0, -1,
		-1, 0,
		-1, 1,
		0, 1
	};
	for (int i=0; i<6; i++) {
		GridPos p(pos.q + offsets[2 * i], pos.r +offsets[2 * i + 1]); 
		if (datas->isPosValid(p)) {
			n.push_back(p);
		}
	}
	return n;
}

unsigned SpatialGrid::ComputeDistance(const GridPos& p1, const GridPos& p2) {
	return (abs(p1.q - p2.q) + abs(p1.r - p2.r)
          + abs(p1.r + p1.q - p2.q - p2.r)) / 2;
}
