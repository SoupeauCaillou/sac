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
    CHECK(desc.load(FB(content)));
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
    CHECK(desc.load(FB(content)));
    CHECK_CLOSE(2.1, desc.playbackSpeed, 0.001);
    CHECK_EQUAL(2, desc.loopCount.t1);
    CHECK_EQUAL(4, desc.loopCount.t2);
    CHECK_EQUAL("animation2", desc.nextAnim);
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
    CHECK(desc.load(FB(content)));
    CHECK_EQUAL(3, desc.textures.size());
    CHECK_EQUAL(desc.textures[0], desc.textures[2]);
    RenderingSystem::DestroyInstance();
}

