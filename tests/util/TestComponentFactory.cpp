#include <UnitTest++.h>
#include "util/ComponentFactory.h"
#include "util/DataFileParser.h"
#include "systems/TransformationSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/ParticuleSystem.h"
#include "base/PlacementHelper.h"
#include "base/EntityManager.h"

struct TestSetup {
    TestSetup() {
        TransformationSystem::CreateInstance();
        ParticuleSystem::CreateInstance();
        AnimationSystem::CreateInstance();
        RenderingSystem::CreateInstance();
    }
    ~TestSetup() {
        theEntityManager.entityTemplateLibrary.unload("test");
        TransformationSystem::DestroyInstance();
        ParticuleSystem::DestroyInstance();
        AnimationSystem::DestroyInstance();
        RenderingSystem::DestroyInstance();
    }
};

struct StubAssetAPI : public AssetAPI {
    std::string data;
    StubAssetAPI(const std::string& pData) : data(pData) {}

    FileBuffer loadAsset(const std::string& ) {
        FileBuffer fb;
        fb.size = data.length() + 1;
        fb.data = new uint8_t[fb.size];
        memcpy(fb.data, data.c_str(), data.length());
        fb.data[fb.size - 1] = '\0';

        return fb;
    }
    std::list<std::string> listContent(const std::string& , const std::string& ) {
        return std::list<std::string>();
    }
    const std::string & getWritableAppDatasPath() {
        static std::string empty("");
        return empty;
    }
};

static Entity doTest(std::string s) {
    StubAssetAPI a(s);
    theEntityManager.entityTemplateLibrary.init(&a, false);
    return theEntityManager.CreateEntity("test", EntityType::Volatile,
        theEntityManager.entityTemplateLibrary.load("test"));
}

TEST_FIXTURE(TestSetup, TestFloatProperty)
{
    Entity e = doTest("[Transformation]\nrotation=1.2");
	CHECK_CLOSE(1.2, TRANSFORM(e)->rotation, 0.001);
}

TEST_FIXTURE(TestSetup, TestFloatFromInterval)
{
    Entity e = doTest("[Transformation]\nrotation=1.2, 3.5");

    CHECK(1.2 <= TRANSFORM(e)->rotation && TRANSFORM(e)->rotation <= 3.5);
}

TEST_FIXTURE(TestSetup, TestFloatIntervalProperty)
{
    Entity e = doTest("[Particule]\nlifetime=1.2, 3.5");

    CHECK_CLOSE(1.2, PARTICULE(e)->lifetime.t1, 0.001);
    CHECK_CLOSE(3.5, PARTICULE(e)->lifetime.t2, 0.001);
}

TEST_FIXTURE(TestSetup, TestSingleVec2Property)
{
    Entity e = doTest("[Transformation]\nsize=2, 4");


    CHECK_CLOSE(2, TRANSFORM(e)->size.x, 0.001);
    CHECK_CLOSE(4, TRANSFORM(e)->size.y, 0.001);
}

TEST_FIXTURE(TestSetup, TestStringProperty)
{
    Entity e = doTest("[Animation]\nname=super_animation");

    CHECK_EQUAL("super_animation", ANIMATION(e)->name);
}

TEST_FIXTURE(TestSetup, TestTextureProperty)
{
    Entity e = doTest("[Rendering]\ntexture=my_texture");

    CHECK_EQUAL(theRenderingSystem.loadTextureFile("my_texture"), RENDERING(e)->texture);
}

TEST_FIXTURE(TestSetup, TestTransformPercentProperty)
{
    PlacementHelper::ScreenWidth = 100;
    PlacementHelper::ScreenHeight = 50;
    Entity e = doTest("[Transformation]\nsize%screen=0.3, 0.2\nposition%screen_w=0.5,0.8");

    CHECK_CLOSE(0.3 * PlacementHelper::ScreenWidth, TRANSFORM(e)->size.x, 0.001);
    CHECK_CLOSE(0.2 *  PlacementHelper::ScreenHeight, TRANSFORM(e)->size.y, 0.001);

    CHECK_CLOSE(0.5 * PlacementHelper::ScreenWidth, TRANSFORM(e)->position.x, 0.001);
    CHECK_CLOSE(0.8 *  PlacementHelper::ScreenWidth, TRANSFORM(e)->position.y, 0.001);
}

TEST_FIXTURE(TestSetup, TestColorProperty)
{
    Entity e = doTest("[Rendering]\ncolor=1.0, 0.5, 0.25, 1");

    CHECK_CLOSE(1.0, RENDERING(e)->color.r, 0.001);
    CHECK_CLOSE(0.5, RENDERING(e)->color.g, 0.001);
    CHECK_CLOSE(0.25, RENDERING(e)->color.b, 0.001);
    CHECK_CLOSE(1, RENDERING(e)->color.a, 0.001);
}

TEST_FIXTURE(TestSetup, TestColorHtmlProperty)
{
    Entity e = doTest("[Rendering]\ncolor%html= 81cadc");

    CHECK_CLOSE(129.0/255, RENDERING(e)->color.r, 0.001);
    CHECK_CLOSE(202.0/255, RENDERING(e)->color.g, 0.001);
    CHECK_CLOSE(220.0/255, RENDERING(e)->color.b, 0.001);
    CHECK_CLOSE(1, RENDERING(e)->color.a, 0.001);
}
