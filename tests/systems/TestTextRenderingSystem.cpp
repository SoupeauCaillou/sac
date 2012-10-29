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

#include "base/MathUtil.h"
#include "systems/TextRenderingSystem.h"
#include <map>

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
