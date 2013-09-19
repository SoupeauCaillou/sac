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



#include <UnitTest++.h>
#include "base/TouchInputManager.h"
#include <glm/glm.hpp>

#if 0
TEST(WindowToWorldCords0)
{
    TransformationComponent tc;
    tc.worldPosition = glm::vec2(0.0f);
    tc.worldRotation = 0.0f;
	glm::vec2 result = theTouchInputManager.windowToWorld(
		glm::vec2(0, 0),
		&tc,);
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
#endif
