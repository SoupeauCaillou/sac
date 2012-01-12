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
