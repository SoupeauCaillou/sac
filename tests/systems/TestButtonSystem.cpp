#include <UnitTest++.h>

#include "systems/ButtonSystem.h"

TEST(pointIsInside)
{
	CHECK(ButtonSystem::inside(Vector2::Zero, Vector2::Zero, Vector2(1, 1)));
	CHECK(ButtonSystem::inside(Vector2(1.5, 0), Vector2(1, -1), Vector2(2, 3)));
}

TEST(pointIsNotInside)
{
	CHECK(!ButtonSystem::inside(Vector2(1.1, 0), Vector2::Zero, Vector2(1, 1)));	
	CHECK(!ButtonSystem::inside(Vector2(0.9, 1.1), Vector2::Zero, Vector2(1, 1)));	
}

