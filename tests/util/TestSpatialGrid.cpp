/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/

#if !DISABLE_GRID_SYSTEM

#include <UnitTest++.h>
#include "util/HexSpatialGrid.h"
#include "util/SquareSpatialGrid.h"

#include <algorithm>

TEST(HexGridNeighborsAtCenter)
{
        HexSpatialGrid grid(5, 5, 1.0f);
        GridPos p(0, 0);
        auto neighbors = grid.getNeighbors(p, false);
        CHECK_EQUAL(6u, neighbors.size());
        for(auto& n: neighbors) {
                CHECK_EQUAL(1u, grid.computeGridDistance(p, n));
        }
}

TEST(HexGridNeighborsAtTopLeft)
{
        HexSpatialGrid grid(5, 5, 1.0f);
        GridPos p(-1,-2);
        auto neighbors = grid.getNeighbors(p, false);
        CHECK_EQUAL(2u, neighbors.size());
        for(auto& n: neighbors) {
                CHECK_EQUAL(1u, grid.computeGridDistance(p, n));
        }
}

TEST(HexPositionToGridPosCenter)
{
    HexSpatialGrid grid(5, 5, 1.0f);
    GridPos p = grid.positionToGridPos(glm::vec2(0, 0));
    CHECK_EQUAL(0, p.q);
    CHECK_EQUAL(0, p.r);
}

TEST(HexRingAt2OfCenter)
{
        HexSpatialGrid grid(5, 5, 1.0f);
        GridPos p = grid.positionToGridPos(glm::vec2(0, 0));
        std::vector<GridPos> i = grid.ringFinder(p, 2, false);
        CHECK_EQUAL(12, (int)i.size());
        for (auto j: i) {
                CHECK_EQUAL(2, (int)grid.computeGridDistance(p, j));
        }
}

TEST(HexLineDrawerWhenStraight)
{
        HexSpatialGrid grid(3, 3, 1.0f);

        auto path = grid.lineDrawer(GridPos(0,0), GridPos(0,1));

        CHECK_EQUAL(2, (int)path.size());
        CHECK_EQUAL(GridPos(0,0), path[0]);
        CHECK_EQUAL(GridPos(0,1), path[1]);
        // CHECK_EQUAL(GridPos(0,2), path[2]);
}

TEST(HexLineDrawerWhenDiagonale)
{
        HexSpatialGrid grid(3, 3, 1.0f);

        auto path = grid.lineDrawer(GridPos(0,0), GridPos(3,3));

        CHECK_EQUAL(7, (int)path.size());
        CHECK_EQUAL(GridPos(0,0), path[0]);
        CHECK_EQUAL(GridPos(1, 0), path[1]);
        CHECK_EQUAL(GridPos(1,1), path[2]);
        CHECK_EQUAL(GridPos(2,1), path[3]);
        CHECK_EQUAL(GridPos(2,2), path[4]);
        CHECK_EQUAL(GridPos(3,2), path[5]);
        CHECK_EQUAL(GridPos(3,3), path[6]);
}

TEST(HexLineDrawerWithNegativeEps)
{
        HexSpatialGrid grid(21, 21, 1.0f);

        auto path = grid.lineDrawer(GridPos(-7,1), GridPos(-5,3), false);

        CHECK_EQUAL(5, (int)path.size());
        CHECK_EQUAL(GridPos(-7,1), path[0]);
        CHECK_EQUAL(GridPos(-7,2), path[1]);
        CHECK_EQUAL(GridPos(-6,2), path[2]);
        CHECK_EQUAL(GridPos(-6,3), path[3]);
        CHECK_EQUAL(GridPos(-5,3), path[4]);
}

TEST(SquareGridNeighborsAtCenter)
{
        SquareSpatialGrid grid(3, 3, 1.0f);
        GridPos p(0, 0);
        auto neighbors = grid.getNeighbors(p, false);
        CHECK_EQUAL(8u, neighbors.size());
        // for(auto& n: neighbors) {
        //      CHECK_EQUAL(1u, grid.computeGridDistance(p, n));
        // }
}

TEST(SquareGridNeighborsAtTopLeft)
{
        SquareSpatialGrid grid(3, 3, 1.0f);
        GridPos p(-1,-1);
        auto neighbors = grid.getNeighbors(p, false);
        CHECK_EQUAL(3u, neighbors.size());
        // for(auto& n: neighbors) {
        //      CHECK_EQUAL(1u, grid.computeGridDistance(p, n));
        // }
}

TEST(SquarePositionToGridPosCenter)
{
    SquareSpatialGrid grid(3, 3, 1.0f);
    GridPos p = grid.positionToGridPos(glm::vec2(0, 0));
    CHECK_EQUAL(0, p.q);
    CHECK_EQUAL(0, p.r);
}

TEST(SquareRingAt2OfCenter)
{
        SquareSpatialGrid grid(7, 7, 1.0f);
        GridPos p = grid.positionToGridPos(glm::vec2(0, 0));
        std::vector<GridPos> i = grid.ringFinder(p, 2, false);
        CHECK_EQUAL(24, (int)i.size());
        // for (auto j: i) {
        //      CHECK_EQUAL(2, (int)grid.computeGridDistance(p, j));
        // }
}

TEST(SquareLineDrawerWhenStraight)
{
        // SquareSpatialGrid grid(3, 3, 1.0f);

        // auto path = grid.lineDrawer(GridPos(0,0), GridPos(0,1));

        // CHECK_EQUAL(2, (int)path.size());
        // CHECK_EQUAL(GridPos(0,0), path[0]);
        // CHECK_EQUAL(GridPos(0,1), path[1]);
        // CHECK_EQUAL(GridPos(0,2), path[2]);
}

TEST(SquareLineDrawerWhenDiagonale)
{
        // SquareSpatialGrid grid(3, 3, 1.0f);

        // auto path = grid.lineDrawer(GridPos(0,0), GridPos(3,3));

        // CHECK_EQUAL(7, (int)path.size());
        // CHECK_EQUAL(GridPos(0,0), path[0]);
        // CHECK_EQUAL(GridPos(1, 0), path[1]);
        // CHECK_EQUAL(GridPos(1,1), path[2]);
        // CHECK_EQUAL(GridPos(2,1), path[3]);
        // CHECK_EQUAL(GridPos(2,2), path[4]);
        // CHECK_EQUAL(GridPos(3,2), path[5]);
        // CHECK_EQUAL(GridPos(3,3), path[6]);
}

TEST(SquareLineDrawerWithNegativeEps)
{
        // SquareSpatialGrid grid(21, 21, 1.0f);

        // auto path = grid.lineDrawer(GridPos(-7,1), GridPos(-5,3), false);

        // CHECK_EQUAL(5, (int)path.size());
        // CHECK_EQUAL(GridPos(-7,1), path[0]);
        // CHECK_EQUAL(GridPos(-7,2), path[1]);
        // CHECK_EQUAL(GridPos(-6,2), path[2]);
        // CHECK_EQUAL(GridPos(-6,3), path[3]);
        // CHECK_EQUAL(GridPos(-5,3), path[4]);
}

#endif
