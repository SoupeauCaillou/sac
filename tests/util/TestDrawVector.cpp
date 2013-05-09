#include <UnitTest++.h>
#include "util/drawVector.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

TEST(DrawVector)
{
	RenderingSystem::CreateInstance();
	TransformationSystem::CreateInstance();

	Entity e = drawVector(glm::vec2(0.f), glm::vec2(1.f));
	
	CHECK_CLOSE(1.4142, TRANSFORM(e)->size.x, 0.0001);
	CHECK_CLOSE(0.7854, TRANSFORM(e)->rotation, 0.0001);

	e = drawVector(glm::vec2(0.f), glm::vec2(-1.f));
	
	CHECK_CLOSE(1.4142, TRANSFORM(e)->size.x, 0.0001);
	CHECK_CLOSE(-2.3562, TRANSFORM(e)->rotation, 0.0001);

	e = drawVector(glm::vec2(0.f), glm::vec2(1.f, -1.f));
	
	CHECK_CLOSE(1.4142, TRANSFORM(e)->size.x, 0.0001);
	CHECK_CLOSE(-0.7854, TRANSFORM(e)->rotation, 0.0001);

	e = drawVector(glm::vec2(0.f), glm::vec2(-1.f, 1.f));
	
	CHECK_CLOSE(1.4142, TRANSFORM(e)->size.x, 0.0001);
	CHECK_CLOSE(2.3562, TRANSFORM(e)->rotation, 0.0001);

	RenderingSystem::DestroyInstance();
	TransformationSystem::DestroyInstance();
}
