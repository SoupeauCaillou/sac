#include <UnitTest++.h>
#include "util/SpatialGrid.h"

#include <algorithm>

TEST(GridNeighborsAtCenter)
{
	SpatialGrid grid(5, 5);
	GridPos p;
	auto neighbors = grid.getNeighbors(p);
	CHECK_EQUAL(6u, neighbors.size());
	for(auto& n: neighbors) {
		CHECK_EQUAL(1u, SpatialGrid::ComputeDistance(p, n));
	}
}