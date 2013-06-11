#include <UnitTest++.h>

#include "util/IntersectionUtil.h"

TEST(parallelLinesCollision)
{
    glm::vec2 intersectionPoint;

    //parallel - no coincident
    CHECK(! IntersectionUtil::lineLine(glm::vec2(0, 0), glm::vec2(1, 2), glm::vec2(0, 1), glm::vec2(1, 3), 0));

    //parallel - coincident - one point in comomn
    CHECK(IntersectionUtil::lineLine(glm::vec2(-1, -1), glm::vec2(1, 2), glm::vec2(-2, -2), glm::vec2(-1, -1), &intersectionPoint));
    CHECK_CLOSE(-1.f, intersectionPoint.x, 0.0001f);
    CHECK_CLOSE(-1.f, intersectionPoint.y, 0.0001f);

    //parallel - coincident - a sub segment in common
    CHECK(IntersectionUtil::lineLine(glm::vec2(3, 4), glm::vec2(3, 6), glm::vec2(3, 5), glm::vec2(3, 7), &intersectionPoint));
    CHECK_CLOSE(3.f, intersectionPoint.x, 0.0001f);
    CHECK_CLOSE(5.f, intersectionPoint.y, 0.0001f);
}
TEST(CoincidentLinesCollisionInclusion)
{
    //parallel - coincident - second segment is within the first segment
    glm::vec2 intersectionPoint;
    CHECK(IntersectionUtil::lineLine(glm::vec2(3, 4), glm::vec2(3, 7), glm::vec2(3, 5), glm::vec2(3, 6), &intersectionPoint));
    CHECK_CLOSE(3.f, intersectionPoint.x, 0.0001f);
    CHECK_CLOSE(5.f, intersectionPoint.y, 0.0001f);
}

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

