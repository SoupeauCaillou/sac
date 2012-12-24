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

