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

#include "base/MathUtil.h"
#include "systems/TransformationSystem.h"

TEST(TransformationPropagationToWorldTransformWithoutParent)
{
	TransformationSystem::CreateInstance();
	Entity e = 1;
	theTransformationSystem.Add(e);
	TransformationComponent* tc = TRANSFORM(e);
	CHECK(tc);
	tc->position = Vector2(4.2, -5.4);
	tc->rotation = 10.4;
	theTransformationSystem.Update(1.0f);
	CHECK_EQUAL(tc->position, tc->worldPosition);
	CHECK_EQUAL(tc->rotation, tc->worldRotation);
	TransformationSystem::DestroyInstance();
}
