#include <UnitTest++.h>
#include "util/SpatialGrid.h"

#include <algorithm>

TEST(GridNeighborsAtCenter)
{
	SpatialGrid grid(5, 5);
	GridPos p(0, 0);
	auto neighbors = grid.getNeighbors(p, false);
	CHECK_EQUAL(6u, neighbors.size());
	for(auto& n: neighbors) {
		CHECK_EQUAL(1u, SpatialGrid::ComputeDistance(p, n));
	}
}

TEST(GridNeighborsAtTopLeft)
{
	SpatialGrid grid(5, 5);
	GridPos p(-1,-2);
	auto neighbors = grid.getNeighbors(p, false);
	CHECK_EQUAL(2u, neighbors.size());
	for(auto& n: neighbors) {
		CHECK_EQUAL(1u, SpatialGrid::ComputeDistance(p, n));
	}
}

TEST(PositionToGridPosCenter)
{
    SpatialGrid grid(5, 5);
    GridPos p = grid.positionToGridPos(glm::vec2(0, 0));
    CHECK_EQUAL(0, p.q);
    CHECK_EQUAL(0, p.r);
}

TEST(RingAt2OfCenter)
{
	SpatialGrid grid(5,5);
	GridPos p = grid.positionToGridPos(glm::vec2(0, 0));
	std::vector<GridPos> i = grid.ringFinder(p, 2, false);
	CHECK_EQUAL(12, (int)i.size());
	for (auto j: i) {
		CHECK_EQUAL(2, (int)grid.ComputeDistance(p, j));
	}
}
