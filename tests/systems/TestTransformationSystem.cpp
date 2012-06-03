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
