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
#include "base/TouchInputManager.h"

TEST(WindowToWorldCords0)
{
	Vector2 result = theTouchInputManager.windowToWorld(
		Vector2(0, 0),
		Vector2(10, 10), Vector2(300, 400));
	Vector2 expected = Vector2(-5, 5);
	CHECK_CLOSE(expected.X, result.X, 0.0001);
	CHECK_CLOSE(expected.Y, result.Y, 0.0001);
}

TEST(WindowToWorldCords1)
{
	Vector2 result = theTouchInputManager.windowToWorld(
		Vector2(150, 200),
		Vector2(10, 10), Vector2(300, 400));
	Vector2 expected = Vector2::Zero;
	CHECK_CLOSE(expected.X, result.X, 0.0001);
	CHECK_CLOSE(expected.Y, result.Y, 0.0001);
}


TEST(WindowToWorldCords2)
{
	Vector2 result = theTouchInputManager.windowToWorld(
		Vector2(250, 280),
		Vector2(10, 20), Vector2(300, 400));
	Vector2 expected(3.3333, -4);
	CHECK_CLOSE(expected.X, result.X, 0.0001);
	CHECK_CLOSE(expected.Y, result.Y, 0.0001);
}
