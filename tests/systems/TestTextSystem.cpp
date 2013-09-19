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
