#include "TimeUtil.h"
#include <time.h>
#include "Log.h"

struct timespec TimeUtil::startup_time;

void TimeUtil::init() {
	// gettimeofday(&startup_time, 0);
    clock_gettime(CLOCK_MONOTONIC, &startup_time);
}

static inline float timeconverter(struct timespec tv) {
	return tv.tv_sec + (float)(tv.tv_nsec) / 1000000000.0f;
}

static inline void sub(struct timespec& tA, const struct timespec& tB)
{
    if ((tA.tv_nsec - tB.tv_nsec) < 0) {
        tA.tv_sec = tA.tv_sec - tB.tv_sec - 1;
        tA.tv_nsec = 1000000000 + tA.tv_nsec - tB.tv_nsec;
    } else {
        tA.tv_sec = tA.tv_sec - tB.tv_sec;
        tA.tv_nsec = tA.tv_nsec - tB.tv_nsec;
    }
}

float TimeUtil::getTime() {
	struct timespec tv;
	if (clock_gettime(CLOCK_MONOTONIC, &tv) != 0) {
        LOGE("clock_gettime failure");
    }
     sub(tv, startup_time);
	return timeconverter(tv);
}
