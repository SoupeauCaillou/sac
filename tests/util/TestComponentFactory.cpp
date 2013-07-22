#include <UnitTest++.h>
#include "util/ComponentFactory.h"
#include "util/DataFileParser.h"
#include "systems/ADSRSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/AnimationSystem.h"
#include "systems/ParticuleSystem.h"
#include "systems/NetworkSystem.h"
#include "base/PlacementHelper.h"
#include "base/EntityManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

struct TestSetup {
    TestSetup() {
        ADSRSystem::CreateInstance();
        TransformationSystem::CreateInstance();
        ParticuleSystem::CreateInstance();
        AnimationSystem::CreateInstance();
        RenderingSystem::CreateInstance();
        NetworkSystem::CreateInstance();
    }
    ~TestSetup() {
        theEntityManager.entityTemplateLibrary.unload("test");
        ADSRSystem::DestroyInstance();
        TransformationSystem::DestroyInstance();
        ParticuleSystem::DestroyInstance();
        AnimationSystem::DestroyInstance();
        RenderingSystem::DestroyInstance();
        NetworkSystem::DestroyInstance();
    }
};

struct StubAssetAPI : public AssetAPI {
    std::string data;
    StubAssetAPI(const std::string& pData) : data(pData) {}

    FileBuffer loadFile(const std::string& ) {
        FileBuffer fb;
        fb.size = data.length() + 1;
        fb.data = new uint8_t[fb.size];
        memcpy(fb.data, data.c_str(), data.length());
        fb.data[fb.size - 1] = '\0';

        return fb;
    }

    FileBuffer loadAsset(const std::string& ) {
        return loadFile("");
    }

    std::list<std::string> listContent(const std::string& , const std::string& , const std::string& ) {
        return std::list<std::string>();
    }
    std::list<std::string> listAssetContent(const std::string& , const std::string& ) {
        return std::list<std::string>();
    }

    const std::string & getWritableAppDatasPath() {
        static std::string empty("");
        return empty;
    }

    void synchronize() {}
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

TEST_FIXTURE(TestSetup, TestVec2PositionProperty)
{
    Entity e = doTest("[Transformation]\n" \
        "positionSE=2, 4\n" \
        "size=2,2");

    CHECK_CLOSE(1, TRANSFORM(e)->position.x, 0.001);
    CHECK_CLOSE(5, TRANSFORM(e)->position.y, 0.001);
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
    PlacementHelper::ScreenSize.x = 100;
    PlacementHelper::ScreenSize.y = 50;
    Entity e = doTest("[Transformation]\nsize%screen=0.3, 0.2\nposition%screen_w=0.5,0.8");

    CHECK_CLOSE(0.3 * PlacementHelper::ScreenSize.x, TRANSFORM(e)->size.x, 0.001);
    CHECK_CLOSE(0.2 *  PlacementHelper::ScreenSize.y, TRANSFORM(e)->size.y, 0.001);

    CHECK_CLOSE(0.5 * PlacementHelper::ScreenSize.x, TRANSFORM(e)->position.x, 0.001);
    CHECK_CLOSE(0.8 *  PlacementHelper::ScreenSize.x, TRANSFORM(e)->position.y, 0.001);
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

TEST_FIXTURE(TestSetup, TestSizeBasedOnTextureRatio)
{
    // define dumb texture
    TextureInfo info;
    info.originalSize = glm::vec2(100, 80);
    theRenderingSystem.textureLibrary.add("plop", info);

    PlacementHelper::ScreenSize.y = 100;
    Entity e = doTest("[Transformation]\n" \
        "size%screen_h,texture_ratio=0.1\n" \
        "[Rendering]\n" \
        "texture=plop");

    CHECK_CLOSE(10, TRANSFORM(e)->size.x, 0.001);
    CHECK_CLOSE(8, TRANSFORM(e)->size.y, 0.001);
}


TEST_FIXTURE(TestSetup, TestStringVector)
{
    Entity e = doTest("[Network]\nsync = abc, defg, z");

    CHECK_EQUAL(3lu, NETWORK(e)->sync.size());
    CHECK_EQUAL("abc", NETWORK(e)->sync[0]);
    CHECK_EQUAL("defg", NETWORK(e)->sync[1]);
    CHECK_EQUAL("z", NETWORK(e)->sync[2]);
}

TEST_FIXTURE(TestSetup, TestGimpVec2Modifier)
{
    PlacementHelper::ScreenSize = glm::vec2(10, 20);
    PlacementHelper::WindowSize = glm::vec2(435, 700);
    PlacementHelper::GimpSize = glm::vec2(800, 1280);

    Entity e = doTest("[Transformation]\nposition%gimp = 10, 20\nsize%gimp = 10, 20");

    CHECK_CLOSE(PlacementHelper::GimpXToScreen(10), TRANSFORM(e)->position.x, 0.001);
    CHECK_CLOSE(PlacementHelper::GimpYToScreen(20), TRANSFORM(e)->position.y, 0.001);
    CHECK_CLOSE(PlacementHelper::GimpWidthToScreen(10), TRANSFORM(e)->size.x, 0.001);
    CHECK_CLOSE(PlacementHelper::GimpHeightToScreen(20), TRANSFORM(e)->size.y, 0.001);
}

TEST_FIXTURE(TestSetup, TestGimpFloatModifier)
{
    PlacementHelper::ScreenSize = glm::vec2(10, 20);
    PlacementHelper::WindowSize = glm::vec2(435, 700);
    PlacementHelper::GimpSize = glm::vec2(800, 1280);

    Entity e = doTest("[ADSR]\nidle_value%gimp_w = 10\nattack_value%gimp_h = 10\nattack_timing%gimp_x = 5\ndecay_timing%gimp_y = 2");
    CHECK_CLOSE(PlacementHelper::GimpWidthToScreen(10), ADSR(e)->idleValue, 0.001);
    CHECK_CLOSE(PlacementHelper::GimpHeightToScreen(10), ADSR(e)->attackValue, 0.001);
    CHECK_CLOSE(PlacementHelper::GimpXToScreen(5), ADSR(e)->attackTiming, 0.001);
    CHECK_CLOSE(PlacementHelper::GimpYToScreen(2), ADSR(e)->decayTiming, 0.001);
}

TEST_FIXTURE(TestSetup, TestDegreeFloatModifier)
{
    Entity e = doTest(  "[Transformation]\n"\
                        "rotation%degrees = 45");

    CHECK_CLOSE(glm::quarter_pi<float>(), TRANSFORM(e)->rotation, 0.001);
}
