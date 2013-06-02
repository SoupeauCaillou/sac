#include "SpatialGrid.h"
#include "base/Log.h"
#include <map>
#include <list>
#include "base/Entity.h"

static uint64_t Hash(const GridPos& p) {
	uint64_t h = p.q;
	h <<= 32;
	return h | p.r;
}

struct Cell {
    std::list<Entity> entities; // <- GRID()
};

GridPos::GridPos(int32_t pQ, int32_t pR) : q(pQ), r(pR) {

}

struct SpatialGrid::SpatialGridData {
	int w, h;
	std::map<uint64_t, Cell> cells;

	SpatialGridData(int pW, int pH) : w(pW), h(pH) {
		// varies on z (r) first
        int qStart = 0;
		for (int z=0; z<=h; z++) {
			// then, compute q
			for (int q=0; q<=w; q++) {
				cells.insert(std::make_pair(Hash(GridPos(q - (qStart >> 1), z)), Cell()));
			}
            qStart++;
		}
	}

	bool isPosValid(const GridPos& pos) const {
        int qStart = (int)(pos.r * -0.5);
        return (pos.r >= 0 && pos.r <= h) &&
            pos.q >= qStart && pos.q <= (qStart + w);
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
