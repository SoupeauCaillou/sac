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
#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

#include <base/EntityManager.h>
#include "systems/TransformationSystem.h"
#include "systems/ADSRSystem.h"

TEST(DeleteSystems)
{
    TransformationSystem::CreateInstance();
    ADSRSystem::CreateInstance();
    Entity e = theEntityManager.CreateEntity(0);
    theEntityManager.AddComponent(e, &theTransformationSystem);
    theEntityManager.AddComponent(e, &theADSRSystem);
    CHECK(theTransformationSystem.Get(e));
    CHECK(theADSRSystem.Get(e));
    theEntityManager.DeleteEntity(e);
    // CHECK_ASSERT(theTransformationSystem.Get(e));
    // CHECK_ASSERT(theADSRSystem.Get(e));
    TransformationSystem::DestroyInstance();
    ADSRSystem::DestroyInstance();
}

TEST(Serialization)
{
    std::cerr << "TestEntityManager.Serialization is BROKEN!!!!!" << std::endl;
    return;
    TransformationSystem::CreateInstance();
    theEntityManager.deleteAllEntities();

    unsigned eCount = (unsigned) glm::linearRand(1.f, 20.f);
    for (unsigned i = 0; i < eCount; ++i) {
        Entity e = theEntityManager.CreateEntity(Murmur::RuntimeHash("d"), EntityType::Persistent);
        ADD_COMPONENT(e, Transformation);
        TRANSFORM(e)->position = glm::vec2(i, i);
    }
    CHECK_EQUAL(eCount, theEntityManager.allEntities().size());


    uint8_t* dump = 0;
    int size = theEntityManager.serialize(&dump);
    CHECK(size != 0);

    theEntityManager.deleteAllEntities();

    theEntityManager.deserialize(dump, size);

    std::vector<Entity> entities = theEntityManager.allEntities();
    CHECK_EQUAL(eCount, entities.size());
    for (unsigned i = 0; i < entities.size(); i++) {
        TransformationComponent* tc = TRANSFORM(entities[i]);
        CHECK(tc != 0);
        CHECK_EQUAL(glm::vec2(i, i).x, tc->position.x);
        CHECK_EQUAL(glm::vec2(i, i).y, tc->position.y);
    }

    delete[] dump;
    TransformationSystem::DestroyInstance();
}
