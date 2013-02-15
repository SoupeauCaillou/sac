#include "TimeUtil.h"
#include <time.h>
#include <glog/logging.h>

#ifndef EMSCRIPTEN
struct timespec TimeUtil::startup_time;
#else
struct timeval TimeUtil::startup_time;
#endif

void TimeUtil::init() {
#ifndef EMSCRIPTEN
    clock_gettime(CLOCK_MONOTONIC, &startup_time);
#else
    gettimeofday(&startup_time, 0);
#endif
}

#ifndef EMSCRIPTEN
static inline float timeconverter(struct timespec tv) {
	return tv.tv_sec + (float)(tv.tv_nsec) / 1000000000.0f;
#else
static inline float timeconverter(struct timeval tv) {
    return (tv.tv_sec + tv.tv_usec / 1000000.0f);
#endif
}

#ifndef EMSCRIPTEN
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
#endif

float TimeUtil::getTime() {
    #ifndef EMSCRIPTEN
	struct timespec tv;
	if (clock_gettime(CLOCK_MONOTONIC, &tv) != 0) {
        LOG(FATAL) << "clock_gettime failure";
    }
     sub(tv, startup_time);
    #else
    struct timeval tv;
    gettimeofday(&tv, 0);
    timersub(&tv, &startup_time, &tv);
    #endif
	return timeconverter(tv);
    
}
