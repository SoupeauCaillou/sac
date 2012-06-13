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
#include "base/EntityManager.h"
#include "base/MathUtil.h"
#include "systems/TransformationSystem.h"
#include "systems/ADSRSystem.h"

TEST(DeleteSystems)
{
	TransformationSystem::CreateInstance();
	ADSRSystem::CreateInstance();
	Entity e = theEntityManager.CreateEntity();
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
	TransformationSystem::CreateInstance();
	theEntityManager.deleteAllEntities();

	int eCount = MathUtil::RandomInt(20) + 1;
	for (int i=0; i<eCount; i++) {
		Entity e = theEntityManager.CreateEntity(EntityManager::Persistent);
		ADD_COMPONENT(e, Transformation);
		TRANSFORM(e)->position = Vector2(i, i);
	}
	CHECK_EQUAL(eCount, theEntityManager.allEntities().size());


	uint8_t* dump = 0;
	int size = theEntityManager.serialize(&dump);
	CHECK(size != 0);

	theEntityManager.deleteAllEntities();

	theEntityManager.deserialize(dump, size);

	std::vector<Entity> entities = theEntityManager.allEntities();
	CHECK_EQUAL(eCount, entities.size());
	for (int i=0; i<entities.size(); i++) {
		TransformationComponent* tc = TRANSFORM(entities[i]);
		CHECK(tc != 0);
		CHECK_EQUAL(Vector2(i, i), tc->position);
	}

	delete[] dump;
	TransformationSystem::DestroyInstance();
}
