#include <UnitTest++.h>

#include "systems/opengl/AnimDescriptor.h"
#include "api/AssetAPI.h"

TEST(LoadSimpleAnimDescriptor)
{
    const char* content = "[meta] " \
        "speed=5.3";
    FileBuffer fb;
    fb.data = (uint8_t*)content;
    fb.size = strlen(content);
    AnimDescriptor desc;
    CHECK(desc.load(fb));
    // CHECK_EQUAL(5.3, desc.playbackSpeed);
}

