#include <UnitTest++.h>
#include "util/ComponentFactory.h"
#include "util/DataFileParser.h"
#include "systems/TransformationSystem.h"
#include "systems/AnimationSystem.h"
#include "base/PlacementHelper.h"

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

TEST(TestTransformPercentProperty)
{
    PlacementHelper::ScreenWidth = 100;
    PlacementHelper::ScreenHeight = 50;
    TransformationSystem::CreateInstance();
    FileBuffer fb = FB("size%screen=0.3, 0.2\nposition%screen_w=0.5,0.8");
    DataFileParser dfp;
    dfp.load(fb);
    TransformationComponent comp;

    CHECK_EQUAL(2, ComponentFactory::build(dfp, "", theTransformationSystem.getSerializer().getProperties(), &comp));

    CHECK_CLOSE(0.3 * PlacementHelper::ScreenWidth, comp.size.x, 0.001);
    CHECK_CLOSE(0.2 *  PlacementHelper::ScreenHeight, comp.size.y, 0.001);

    CHECK_CLOSE(0.5 * PlacementHelper::ScreenWidth, comp.position.x, 0.001);
    CHECK_CLOSE(0.8 *  PlacementHelper::ScreenWidth, comp.position.y, 0.001);
    TransformationSystem::DestroyInstance();
}

TEST(TestColorProperty)
{
    RenderingSystem::CreateInstance();
    FileBuffer fb = FB("color=1.0, 0.5, 0.25, 1");
    DataFileParser dfp;
    dfp.load(fb);
    RenderingComponent comp;

    CHECK_EQUAL(1, ComponentFactory::build(dfp, "", theRenderingSystem.getSerializer().getProperties(), &comp));

    CHECK_CLOSE(1.0, comp.color.r, 0.001);
    CHECK_CLOSE(0.5, comp.color.g, 0.001);
    CHECK_CLOSE(0.25, comp.color.b, 0.001);
    CHECK_CLOSE(1, comp.color.a, 0.001);
    RenderingSystem::DestroyInstance();
}
