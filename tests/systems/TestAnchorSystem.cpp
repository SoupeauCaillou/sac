#include <UnitTest++.h>

#include <glm/gtx/vector_angle.hpp>
#include <glm/gtc/constants.hpp>
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

static const std::vector<glm::vec2> rectanglePoints(const TransformationComponent* tc1) {
    std::vector<glm::vec2> res;

    //NW
    res.push_back( tc1->position + glm::rotate(glm::vec2(- tc1->size.x, tc1->size.y) * .5f, tc1->rotation));
    //NE
    res.push_back( tc1->position + glm::rotate(glm::vec2(tc1->size.x, tc1->size.y) * .5f, tc1->rotation));
    //SW
    res.push_back( tc1->position + glm::rotate(glm::vec2(- tc1->size.x, - tc1->size.y) * .5f, tc1->rotation));
    //SE
    res.push_back( tc1->position + glm::rotate(glm::vec2(tc1->size.x, - tc1->size.y) * .5f, tc1->rotation));

    return res;
}

TEST(TransformationTestCentralRotation90Degrees) {
    AnchorSystem::CreateInstance();
    TransformationSystem::CreateInstance();
    Entity e = 1;
    theTransformationSystem.Add(e);
    theAnchorSystem.Add(e);

    TRANSFORM(e)->position = glm::vec2(1, 0);
    TRANSFORM(e)->size = glm::vec2(2, 1);

    ANCHOR(e)->parent = e;
    ANCHOR(e)->rotation = 0.5f * glm::pi<float>();
    theTransformationSystem.Update(1.0f);
    theAnchorSystem.Update(1.0f);

    glm::vec2 expected[4] = { glm::vec2(.5,-1), glm::vec2(.5, 1),
                              glm::vec2(1.5, -1), glm::vec2(1.5, 1) };

    auto values = rectanglePoints(TRANSFORM(e));

    for (int i = 0; i < 4; ++i) {
       CHECK_CLOSE(expected[i].x, values[i].x, 0.0001);
       CHECK_CLOSE(expected[i].y, values[i].y, 0.0001);
    }


    AnchorSystem::DestroyInstance();
    TransformationSystem::DestroyInstance();
}

TEST(TransformationTestNorthWestRotation90Degrees) {
    TransformationSystem::CreateInstance();
    AnchorSystem::CreateInstance();
    Entity e = 1;
    theTransformationSystem.Add(e);
    theAnchorSystem.Add(e);

    TRANSFORM(e)->position = glm::vec2(1, 0);
    TRANSFORM(e)->size = glm::vec2(2, 1);

    ANCHOR(e)->parent = e;
    ANCHOR(e)->position = glm::vec2(-1, .5);
    ANCHOR(e)->rotation = .5f * glm::pi<float>();

    theTransformationSystem.Update(1.0f);
    theAnchorSystem.Update(1.0f);

    glm::vec2 expected[4] = { glm::vec2(0, .5), glm::vec2(0, 2.5),
                              glm::vec2(1, 0.5), glm::vec2(1, 2.5) };

    auto values = rectanglePoints(TRANSFORM(e));

    for (int i = 0; i < 4; ++i) {
       CHECK_CLOSE(expected[i].x, values[i].x, 0.0001);
       CHECK_CLOSE(expected[i].y, values[i].y, 0.0001);
    }


    AnchorSystem::DestroyInstance();
    TransformationSystem::DestroyInstance();
}

TEST(TransformationTestSouthRotation45Degrees) {
    AnchorSystem::CreateInstance();
    TransformationSystem::CreateInstance();
    Entity e = 1;
    theTransformationSystem.Add(e);
    theAnchorSystem.Add(e);

    TRANSFORM(e)->position = glm::vec2(1, 0);
    TRANSFORM(e)->size = glm::vec2(2, 1);

    ANCHOR(e)->parent = e;
    ANCHOR(e)->position = glm::vec2(0, -0.5);
    ANCHOR(e)->rotation = .25f * glm::pi<float>();

    theTransformationSystem.Update(1.0f);
    theAnchorSystem.Update(1.0f);

    glm::vec2 expected[4] = { glm::vec2(-.41, -.5), glm::vec2(1, 0.91),
                              glm::vec2(0.29, -1.2), glm::vec2(1.7, 0.20) };

    auto values = rectanglePoints(TRANSFORM(e));

    for (int i = 0; i < 4; ++i) {
       CHECK_CLOSE(expected[i].x, values[i].x, 0.01);
       CHECK_CLOSE(expected[i].y, values[i].y, 0.01);
    }


    AnchorSystem::DestroyInstance();
    TransformationSystem::DestroyInstance();
}
