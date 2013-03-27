#include <UnitTest++.h>

#include <glm/glm.hpp>
#include "systems/TransformationSystem.h"

TEST(TransformationPropagationToWorldTransformWithoutParent)
{
	TransformationSystem::CreateInstance();
	Entity e = 1;
	theTransformationSystem.Add(e);
	TransformationComponent* tc = TRANSFORM(e);
	CHECK(tc);
	tc->position = glm::vec2(4.2, -5.4);
	tc->rotation = 10.4;
	theTransformationSystem.Update(1.0f);
	CHECK_EQUAL(tc->position.x, tc->worldPosition.x);
    CHECK_EQUAL(tc->position.y, tc->worldPosition.y);

    CHECK_EQUAL(tc->position.x, tc->worldPosition.x);
    CHECK_EQUAL(tc->position.y, tc->worldPosition.y);

	TransformationSystem::DestroyInstance();
}

TEST(TransformationParentingChain)
{
    TransformationSystem::CreateInstance();
    Entity e[3];
    for (int i=0; i<3; i++) {
        e[i] = i + 1;
        theTransformationSystem.Add(e[i]);
        TRANSFORM(e[i])->position = glm::vec2(1, 0);
        if (i > 0)
            TRANSFORM(e[i])->parent = e[i-1];
    }
    theTransformationSystem.Update(1.0f);
    CHECK_CLOSE(1, TRANSFORM(e[0])->worldPosition.x, 0.001);
    CHECK_CLOSE(2, TRANSFORM(e[1])->worldPosition.x, 0.001);
    CHECK_CLOSE(3, TRANSFORM(e[2])->worldPosition.x, 0.001);
    TransformationSystem::DestroyInstance();
}
