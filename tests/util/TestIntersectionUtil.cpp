/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <UnitTest++.h>

#include "util/IntersectionUtil.h"

TEST(pointIsInside)
{
	CHECK(IntersectionUtil::pointRectangle(Vector2::Zero, Vector2::Zero, Vector2(1, 1)));
	CHECK(IntersectionUtil::pointRectangle(Vector2(1.5, 0), Vector2(1, -1), Vector2(2, 3)));
}

TEST(pointIsNotInside)
{
	CHECK(!IntersectionUtil::pointRectangle(Vector2(1.1, 0), Vector2::Zero, Vector2(1, 1)));	
	CHECK(!IntersectionUtil::pointRectangle(Vector2(0.9, 1.1), Vector2::Zero, Vector2(1, 1)));	
}

