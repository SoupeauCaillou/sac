#include <UnitTest++.h>
#include "base/TouchInputManager.h"
#include <glm/glm.hpp>

TEST(WindowToWorldCords0)
{
	glm::vec2 result = theTouchInputManager.windowToWorld(
		glm::vec2(0, 0),
		glm::vec2(10, 10), glm::vec2(300, 400));
	glm::vec2 expected = glm::vec2(-5, 5);
	CHECK_CLOSE(expected.x, result.x, 0.0001);
	CHECK_CLOSE(expected.y, result.y, 0.0001);
}

TEST(WindowToWorldCords1)
{
	glm::vec2 result = theTouchInputManager.windowToWorld(
		glm::vec2(150, 200),
		glm::vec2(10, 10), glm::vec2(300, 400));
	glm::vec2 expected = glm::vec2(0, 0);
	CHECK_CLOSE(expected.x, result.x, 0.0001);
	CHECK_CLOSE(expected.y, result.y, 0.0001);
}


TEST(WindowToWorldCords2)
{
	glm::vec2 result = theTouchInputManager.windowToWorld(
		glm::vec2(250, 280),
		glm::vec2(10, 20), glm::vec2(300, 400));
	glm::vec2 expected(3.3333, -4);
	CHECK_CLOSE(expected.x, result.x, 0.0001);
	CHECK_CLOSE(expected.y, result.y, 0.0001);
}
