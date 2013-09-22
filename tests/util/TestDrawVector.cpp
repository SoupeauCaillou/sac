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
#include "util/DrawSomething.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

TEST(DrawVector)
{
	RenderingSystem::CreateInstance();
	TransformationSystem::CreateInstance();

	Entity e = DrawSomething::DrawVec2("test", glm::vec2(0.f), glm::vec2(1.f));

	CHECK_CLOSE(1.4142, TRANSFORM(e)->size.x, 0.0001);
	CHECK_CLOSE(0.7854, TRANSFORM(e)->rotation, 0.0001);

	e = DrawSomething::DrawVec2("test", glm::vec2(0.f), glm::vec2(-1.f));

	CHECK_CLOSE(1.4142, TRANSFORM(e)->size.x, 0.0001);
	CHECK_CLOSE(-2.3562, TRANSFORM(e)->rotation, 0.0001);

	e = DrawSomething::DrawVec2("test", glm::vec2(0.f), glm::vec2(1.f, -1.f));

	CHECK_CLOSE(1.4142, TRANSFORM(e)->size.x, 0.0001);
	CHECK_CLOSE(-0.7854, TRANSFORM(e)->rotation, 0.0001);

	e = DrawSomething::DrawVec2("test", glm::vec2(0.f), glm::vec2(-1.f, 1.f));

	CHECK_CLOSE(1.4142, TRANSFORM(e)->size.x, 0.0001);
	CHECK_CLOSE(2.3562, TRANSFORM(e)->rotation, 0.0001);

    DrawSomething::Clear();

	RenderingSystem::DestroyInstance();
	TransformationSystem::DestroyInstance();
}
