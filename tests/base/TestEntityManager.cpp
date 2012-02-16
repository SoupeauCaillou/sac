#include <UnitTest++.h>
#include "base/EntityManager.h"
#include "systems/TransformationSystem.h"
#include "systems/ADSRSystem.h"

TEST(DeleteSystems)
{
	Entity e = theEntityManager.CreateEntity();
	theEntityManager.AddComponent(e, &theTransformationSystem);
	theEntityManager.AddComponent(e, &theADSRSystem);
	CHECK(theTransformationSystem.Get(e));	
	CHECK(theADSRSystem.Get(e));
	theEntityManager.DeleteEntity(e);
	CHECK(theTransformationSystem.Get(e) == NULL);	
	CHECK(theADSRSystem.Get(e) == NULL);
}

