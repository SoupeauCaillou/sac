#include <UnitTest++.h>

#include <glm/glm.hpp>
#include "systems/AnchorSystem.h"
#include "systems/TransformationSystem.h"

TEST(AnchorParentingChain)
{
    TransformationSystem::CreateInstance();
    AnchorSystem::CreateInstance();
    Entity e[3];
    for (int i=0; i<3; i++) {
        e[i] = i + 1;
        theTransformationSystem.Add(e[i]);
        theAnchorSystem.Add(e[i]);
        glm::vec2 position = glm::vec2(1, 0);
        if (i > 0) {
            ANCHOR(e[i])->position = position;
            ANCHOR(e[i])->parent = e[i-1];
        } else {
            TRANSFORM(e[i])->position = position;
        }
    }
    theAnchorSystem.Update(1.0f);
    CHECK_CLOSE(1, TRANSFORM(e[0])->position.x, 0.001);
    CHECK_CLOSE(2, TRANSFORM(e[1])->position.x, 0.001);
    CHECK_CLOSE(3, TRANSFORM(e[2])->position.x, 0.001);
    TransformationSystem::DestroyInstance();
    AnchorSystem::DestroyInstance();
}
