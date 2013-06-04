#include "SpatialGrid.h"
#include "base/Log.h"
#include <map>
#include <list>
#include "base/Entity.h"

static GridPos cubeCoordinateRounding(float x, float y, float z);
static GridPos positionSizeToGridPos(const glm::vec2& pos, float size);
static uint64_t Hash(const GridPos& p) {
	uint64_t h = (uint64_t)p.q;
	h <<= 32;
	return h | (uint32_t)p.r;
}

struct Cell {
    std::list<Entity> entities; // <- GRID()
};

GridPos::GridPos(int32_t pQ, int32_t pR) : q(pQ), r(pR) {

}

struct SpatialGrid::SpatialGridData {
	int w, h;
    float size;
	std::map<uint64_t, Cell> cells;

	SpatialGridData(int pW, int pH, float hexagonWidth) : w(pW), h(pH) {
        LOGF_IF((h % 2) == 0, "Must use odd height");
        LOGF_IF((w % 2) == 0, "Must use odd width");

        float hexaHeight = hexagonWidth / (glm::sqrt(3.0f) * 0.5);
        size = hexaHeight * 0.5;

        const float vertSpacing = 3.0f/4 * hexaHeight;
        const float horiSpacing = hexagonWidth;

        GridPos endCell, firstCell = positionSizeToGridPos(
            glm::vec2(horiSpacing * (int)(-h*0.5f), vertSpacing * (int)(-w*0.5f)), size);

		// varies on z (r) first
        int qStart = firstCell.q;
		for (int z=0; z < h; z++) {
			// then, compute q
			for (int q=0; q<w; q++) {
                endCell = GridPos(qStart + q, firstCell.r + z);
				cells.insert(std::make_pair(Hash(endCell), Cell()));
			}
            if ((z % 2) == 1) {
                qStart--;
            }
		}
        LOGF_IF((endCell.q != -firstCell.q) || (endCell.r != -firstCell.r), "Incoherent first/last cell");
	}

	bool isPosValid(const GridPos& pos) const {
        return cells.find(Hash(pos)) != cells.end();
	}

};

SpatialGrid::SpatialGrid(int w, int h, float hexagonWidth) {
	datas = new SpatialGridData(w, h, hexagonWidth);
}

glm::vec2 SpatialGrid::gridPosToPosition(const GridPos& gp) const {
    return glm::vec2(
        datas->size * glm::sqrt(3.0f) * (gp.q + gp.r * 0.5),
        datas->size * 3.0f * 0.5 * gp.r);
}

GridPos SpatialGrid::positionToGridPos(const glm::vec2& pos) const {
    return positionSizeToGridPos(pos, datas->size);
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

static GridPos cubeCoordinateRounding(float x, float y, float z) {
    float rx = glm::round(x);
    float ry = glm::round(y);
    float rz = glm::round(z);
    float x_err = glm::abs(rx - x);
    float y_err = glm::abs(ry - y);
    float z_err = glm::abs(rz - z);

    if (x_err > y_err && x_err > z_err) {
        rx = -ry - rz;
    } else if (y_err > z_err) {
        ry = -rx - rz;
    } else {
        rz = -rx - ry;
    }
    return GridPos(rx, rz);
}

static GridPos positionSizeToGridPos(const glm::vec2& pos, float size) {
    // the center of the grid is at 0,0
    float q = (1.0f/3 * glm::sqrt(3.0f) * pos.x - 1.0f/3 * pos.y) / size;
    float r = 2.0f/3 * pos.y / size;

    return cubeCoordinateRounding(q, 0 - (q + r), r);
}
