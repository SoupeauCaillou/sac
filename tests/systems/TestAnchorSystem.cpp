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
    Entity e = 1, f = 2;
    theTransformationSystem.Add(f);
    theTransformationSystem.Add(e);
    theAnchorSystem.Add(e);

    TRANSFORM(e)->size = glm::vec2(2, 1);

    ANCHOR(e)->position = glm::vec2(1, 0);
    ANCHOR(e)->parent = f;
    ANCHOR(e)->rotation = 0.5f * glm::pi<float>();

    theAnchorSystem.Update(1.0f);

    CHECK_CLOSE(1, TRANSFORM(e)->position.x, 0.001);
    CHECK_CLOSE(0, TRANSFORM(e)->position.y, 0.001);

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
    Entity e = 1, f = 2;
    theTransformationSystem.Add(f);
    theTransformationSystem.Add(e);
    theAnchorSystem.Add(e);

    TRANSFORM(e)->size = glm::vec2(2, 1);

    ANCHOR(e)->position = glm::vec2(1, 0);
    ANCHOR(e)->anchor = glm::vec2(-1, .5);
    ANCHOR(e)->parent = f;
    ANCHOR(e)->rotation = .5f * glm::pi<float>();

    theAnchorSystem.Update(1.0f);

    CHECK_CLOSE(0.5, TRANSFORM(e)->position.x, 0.001);
    CHECK_CLOSE(1.5, TRANSFORM(e)->position.y, 0.001);

    AnchorSystem::DestroyInstance();
    TransformationSystem::DestroyInstance();
}

TEST(TransformationTestSouthRotation45Degrees) {
    AnchorSystem::CreateInstance();
    TransformationSystem::CreateInstance();
    Entity e = 1, f = 2;
    theTransformationSystem.Add(e);
    theTransformationSystem.Add(f);
    theAnchorSystem.Add(e);

    TRANSFORM(e)->size = glm::vec2(2, 1);

    ANCHOR(e)->position = glm::vec2(1, 0);
    ANCHOR(e)->anchor = glm::vec2(0, -0.5);
    ANCHOR(e)->rotation = .25f * glm::pi<float>();
    ANCHOR(e)->parent = f;

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
