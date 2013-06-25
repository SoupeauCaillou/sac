#include <UnitTest++.h>

#include <glm/glm.hpp>
#include "systems/TextSystem.h"
#include <map>

#if 0
TEST(ComputeSimpleTextLength)
{
    std::map<unsigned char, float> charH2Wratio;
    charH2Wratio['a'] = 1;
    TextRenderingSystem::CreateInstance();
    theTextRenderingSystem.registerFont("dummy", charH2Wratio);
    TextRenderingComponent trc;
    trc.text = "aaaaa";
    trc.fontName = "dummy";
    trc.charHeight = 1;
    CHECK_EQUAL(5, theTextRenderingSystem.computeTextRenderingComponentWidth(&trc));
    TextRenderingSystem::DestroyInstance();
}

TEST(ComputeTextWithInlineImageLength)
{
    std::map<unsigned char, float> charH2Wratio;
    charH2Wratio['a'] = 1;
    TextRenderingSystem::CreateInstance();
    theTextRenderingSystem.registerFont("dummy", charH2Wratio);
    TextRenderingComponent trc;
    trc.text = "aaaaa×texture_name,2.3,3.4×aaa";
    trc.fontName = "dummy";
    trc.charHeight = 1;
    CHECK_CLOSE(5 + 2.3 + 3, theTextRenderingSystem.computeTextRenderingComponentWidth(&trc), 0.001);
    TextRenderingSystem::DestroyInstance();
}
#endif
