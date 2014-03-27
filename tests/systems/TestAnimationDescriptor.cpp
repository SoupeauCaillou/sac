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

#include "systems/opengl/AnimDescriptor.h"
#include "api/AssetAPI.h"
#include "systems/RenderingSystem.h"

static FileBuffer FB(const char* content) {
    FileBuffer fb;
    fb.data = (uint8_t*)content;
    fb.size = strlen(content);
    return fb;
}

TEST(LoadSimpleAnimDescriptor)
{
    const char* content = "[meta]\n" \
        "speed=5.3";
    AnimDescriptor desc;
    CHECK(desc.load(__FUNCTION__, FB(content)));
    CHECK_CLOSE(5.3, desc.playbackSpeed, 0.001);
}

TEST(LoadMetaInfo)
{
    const char* content = "[meta]\n" \
        "speed=2.1\n" \
        "loop=2,4\n" \
        "next_anim=animation2\n" \
        "wait_before_next_anim=0,4.5\n";
    AnimDescriptor desc;
    CHECK(desc.load(__FUNCTION__, FB(content)));
    CHECK_CLOSE(2.1, desc.playbackSpeed, 0.001);
    CHECK_EQUAL(2, desc.loopCount.t1);
    CHECK_EQUAL(4, desc.loopCount.t2);
    CHECK_EQUAL(Murmur::RuntimeHash("animation2"), desc.nextAnim);
    CHECK_CLOSE(0, desc.nextAnimWait.t1, 0.001);
    CHECK_CLOSE(4.5, desc.nextAnimWait.t2, 0.001);
}

TEST(LoadFramesTexture)
{
    RenderingSystem::CreateInstance();
    const char* content = "[meta]\n" \
        "num_frames=3\n" \
        "[frame0]\n" \
        "texture=plop\n" \
        "[frame1]\n" \
        "texture=zou\n" \
        "[frame2]\n" \
        "texture=plop\n";
    AnimDescriptor desc;
    CHECK(desc.load(__FUNCTION__, FB(content)));
    CHECK_EQUAL((unsigned)3, desc.frames.size());
    CHECK_EQUAL(desc.frames[0].texture, desc.frames[2].texture);
    RenderingSystem::DestroyInstance();
}
