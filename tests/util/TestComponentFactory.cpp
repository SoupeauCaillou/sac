#include <UnitTest++.h>
#include "util/ComponentFactory.h"
#include "util/DataFileParser.h"
#include "systems/TransformationSystem.h"
#include "systems/AnimationSystem.h"

static FileBuffer FB(const char* str) {
    FileBuffer fb;
    fb.data = (uint8_t*) str;
    fb.size = strlen(str);
    return fb;
}

TEST(TestFloatProperty)
{
	TransformationSystem::CreateInstance();
	FileBuffer fb = FB("rotation=1.2");
	DataFileParser dfp;
	dfp.load(fb);
	TransformationComponent comp;

	CHECK_EQUAL(1, ComponentFactory::build(dfp, "", theTransformationSystem.getSerializer().getProperties(), &comp));

	CHECK_CLOSE(1.2, comp.rotation, 0.001);
	TransformationSystem::DestroyInstance();
}

TEST(TestFloatIntervalProperty)
{
	TransformationSystem::CreateInstance();
	FileBuffer fb = FB("rotation=1.2, 3.5");
	DataFileParser dfp;
	dfp.load(fb);
	TransformationComponent comp;

	CHECK_EQUAL(1, ComponentFactory::build(dfp, "", theTransformationSystem.getSerializer().getProperties(), &comp));
	CHECK(1.2 <= comp.rotation && comp.rotation <= 3.5);
	TransformationSystem::DestroyInstance();
}


TEST(TestSingleVec2Property)
{
	TransformationSystem::CreateInstance();
	FileBuffer fb = FB("size=2, 4");
	DataFileParser dfp;
	dfp.load(fb);
	TransformationComponent comp;

	CHECK_EQUAL(1, ComponentFactory::build(dfp, "", theTransformationSystem.getSerializer().getProperties(), &comp));

	CHECK_CLOSE(2, comp.size.x, 0.001);
	CHECK_CLOSE(4, comp.size.y, 0.001);
	TransformationSystem::DestroyInstance();
}

TEST(TestStringProperty)
{
	AnimationSystem::CreateInstance();
	FileBuffer fb = FB("name=super_animation");
	DataFileParser dfp;
	dfp.load(fb);
	AnimationComponent comp;

	CHECK_EQUAL(1, ComponentFactory::build(dfp, "", theAnimationSystem.getSerializer().getProperties(), &comp));

	CHECK_EQUAL("super_animation", comp.name);
	AnimationSystem::DestroyInstance();
}

TEST(TestTextureProperty)
{
	RenderingSystem::CreateInstance();
	FileBuffer fb = FB("texture=my_texture");
	DataFileParser dfp;
	dfp.load(fb);
	RenderingComponent comp;

	CHECK_EQUAL(1, ComponentFactory::build(dfp, "", theRenderingSystem.getSerializer().getProperties(), &comp));

	CHECK_EQUAL(theRenderingSystem.loadTextureFile("my_texture"), comp.texture);
	RenderingSystem::DestroyInstance();
}