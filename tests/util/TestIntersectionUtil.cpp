#include <UnitTest++.h>

#include "util/IntersectionUtil.h"

TEST(pointIsInside)
{
	CHECK(IntersectionUtil::pointRectangle(glm::vec2(0, 0), glm::vec2(0, 0), glm::vec2(1, 1)));
	CHECK(IntersectionUtil::pointRectangle(glm::vec2(1.5, 0), glm::vec2(1, -1), glm::vec2(2, 3)));
}

TEST(pointIsNotInside)
{
	CHECK(!IntersectionUtil::pointRectangle(glm::vec2(1.1, 0), glm::vec2(0, 0), glm::vec2(1, 1)));
	CHECK(!IntersectionUtil::pointRectangle(glm::vec2(0.9, 1.1), glm::vec2(0, 0), glm::vec2(1, 1)));
}

