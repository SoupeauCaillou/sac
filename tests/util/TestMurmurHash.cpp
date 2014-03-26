#include <UnitTest++.h>
#include "util/MurmurHash.h"

// simple tests to make sure constexpr version matches previous version output
TEST(ConstExprHash)
{
    CHECK_EQUAL(Murmur::RuntimeHash("test", 4), Murmur::Hash("test"));
    CHECK_EQUAL(Murmur::RuntimeHash("assets/hdpi/texture.png", strlen("assets/hdpi/texture.png")), Murmur::Hash("assets/hdpi/texture.png"));
    CHECK_EQUAL(Murmur::RuntimeHash("super_animation", strlen("super_animation")), Murmur::Hash("super_animation"));
}
