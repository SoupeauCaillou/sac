#include <UnitTest++.h>
#include "util/SpatialGrid.h"

#include <algorithm>

TEST(GridNeighborsAtCenter)
{
	SpatialGrid grid(6, 6);
	GridPos p(1, 2);
	auto neighbors = grid.getNeighbors(p);
	CHECK_EQUAL(6u, neighbors.size());
	for(auto& n: neighbors) {
		CHECK_EQUAL(1u, SpatialGrid::ComputeDistance(p, n));
	}
}

TEST(GridNeighborsAtTopLeft)
{
	SpatialGrid grid(6, 6);
	GridPos p(-0, 0);
	auto neighbors = grid.getNeighbors(p);
	CHECK_EQUAL(2u, neighbors.size());
	for(auto& n: neighbors) {
		CHECK_EQUAL(1u, SpatialGrid::ComputeDistance(p, n));
	}
}
