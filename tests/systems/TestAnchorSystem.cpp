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

#include "util/Random.h"

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

TEST(TransformationTestCentralRotationRandomDegrees) {
    TransformationSystem::CreateInstance();
    AnchorSystem::CreateInstance();
    Entity e = 1, f = 2;
    // This two entities are position vector and anchor vector of children anchor
    // These must be in same position
    Entity ref1 = 3, ref2 = 4;
    theTransformationSystem.Add(f);
    theTransformationSystem.Add(e);
    theAnchorSystem.Add(e);
    
    theTransformationSystem.Add(ref1);
    theTransformationSystem.Add(ref2);
    theAnchorSystem.Add(ref1);
    theAnchorSystem.Add(ref2);

    TRANSFORM(e)->size = glm::vec2(2, 1);

    ANCHOR(e)->parent = f;
    // Anchor on C
    ANCHOR(e)->anchor = glm::vec2(0);

    ANCHOR(ref1)->parent = f; // it's position vector
    ANCHOR(ref2)->parent = e; // it's anchor vector
    ANCHOR(ref2)->anchor = -ANCHOR(e)->anchor;

    // We can do a random number of test there
    for (int i=0; i<50; ++i) {    
        ANCHOR(e)->position = glm::vec2(Random::Float(0, 10), Random::Float(0, 10));
        ANCHOR(e)->rotation = Random::Float(-2, 2) * glm::pi<float>();

        ANCHOR(ref1)->position = ANCHOR(e)->position;

        theAnchorSystem.Update(1.0f);
        
        CHECK_CLOSE(TRANSFORM(ref1)->position.x, TRANSFORM(ref2)->position.x, 0.001);
        CHECK_CLOSE(TRANSFORM(ref1)->position.y, TRANSFORM(ref2)->position.y, 0.001);
    }

    AnchorSystem::DestroyInstance();
    TransformationSystem::DestroyInstance();
}

TEST(TransformationTestNorthWestRotationRandomDegrees) {
    TransformationSystem::CreateInstance();
    AnchorSystem::CreateInstance();
    Entity e = 1, f = 2;
    // This two entities are position vector and anchor vector of children anchor
    // These must be in same position
    Entity ref1 = 3, ref2 = 4;
    theTransformationSystem.Add(f);
    theTransformationSystem.Add(e);
    theAnchorSystem.Add(e);
    
    theTransformationSystem.Add(ref1);
    theTransformationSystem.Add(ref2);
    theAnchorSystem.Add(ref1);
    theAnchorSystem.Add(ref2);

    TRANSFORM(e)->size = glm::vec2(2, 1);

    ANCHOR(e)->parent = f;
    // Anchor on NW
    ANCHOR(e)->anchor = glm::vec2(-1, 0.5);

    ANCHOR(ref1)->parent = f; // it's position vector
    ANCHOR(ref2)->parent = e; // it's anchor vector
    ANCHOR(ref2)->anchor = -ANCHOR(e)->anchor;

    // We can do a random number of test there
    for (int i=0; i<50; ++i) {    
        ANCHOR(e)->position = glm::vec2(Random::Float(0, 10), Random::Float(0, 10));
        ANCHOR(e)->rotation = Random::Float(-2, 2) * glm::pi<float>();

        ANCHOR(ref1)->position = ANCHOR(e)->position;

        theAnchorSystem.Update(1.0f);
        
        CHECK_CLOSE(TRANSFORM(ref1)->position.x, TRANSFORM(ref2)->position.x, 0.001);
        CHECK_CLOSE(TRANSFORM(ref1)->position.y, TRANSFORM(ref2)->position.y, 0.001);
    }

    AnchorSystem::DestroyInstance();
    TransformationSystem::DestroyInstance();
}

TEST(TransformationTestSouthRotationRandomDegrees) {
 TransformationSystem::CreateInstance();
    AnchorSystem::CreateInstance();
    Entity e = 1, f = 2;
    // This two entities are position vector and anchor vector of children anchor
    // These must be in same position
    Entity ref1 = 3, ref2 = 4;
    theTransformationSystem.Add(f);
    theTransformationSystem.Add(e);
    theAnchorSystem.Add(e);
    
    theTransformationSystem.Add(ref1);
    theTransformationSystem.Add(ref2);
    theAnchorSystem.Add(ref1);
    theAnchorSystem.Add(ref2);

    TRANSFORM(e)->size = glm::vec2(2, 1);

    ANCHOR(e)->parent = f;
    // Anchor on S
    ANCHOR(e)->anchor = glm::vec2(0, -0.5);
    
    ANCHOR(ref1)->parent = f; // it's position vector
    ANCHOR(ref2)->parent = e; // it's anchor vector
    ANCHOR(ref2)->anchor = -ANCHOR(e)->anchor;

    // We can do a random number of test there
    for (int i=0; i<50; ++i) {    
        ANCHOR(e)->position = glm::vec2(Random::Float(0, 10), Random::Float(0, 10));
        ANCHOR(e)->rotation = Random::Float(-2, 2) * glm::pi<float>();

        ANCHOR(ref1)->position = ANCHOR(e)->position;

        theAnchorSystem.Update(1.0f);
        
        CHECK_CLOSE(TRANSFORM(ref1)->position.x, TRANSFORM(ref2)->position.x, 0.001);
        CHECK_CLOSE(TRANSFORM(ref1)->position.y, TRANSFORM(ref2)->position.y, 0.001);
    }

    AnchorSystem::DestroyInstance();
    TransformationSystem::DestroyInstance();
}
