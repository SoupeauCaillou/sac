#include <UnitTest++.h>
#include <base/TimeUtil.h>

TEST(TimeForward)
{
    TimeUtil::Init();
    float previousT = TimeUtil::GetTime();

    for (int i = 0 ;i<10000; ++i) {
        float t = TimeUtil::GetTime();
        CHECK(t >= previousT);
        previousT = t;
    }
}
