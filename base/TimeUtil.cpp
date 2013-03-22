#include "TimeUtil.h"
#include <time.h>

#include <glog/logging.h>


#ifdef EMSCRIPTEN
	struct timeval TimeUtil::startup_time;
#elif defined(LINUX)
	struct timespec TimeUtil::startup_time;
#elif defined(WINDOWS)
	__int64 TimeUtil::startup_time;
	float TimeUtil::frequency;
#endif

void TimeUtil::Init() {
#ifdef EMSCRIPTEN
    gettimeofday(&startup_time, 0);
#elif defined(LINUX)
	clock_gettime(CLOCK_MONOTONIC, &startup_time);    
#elif defined(WINDOWS)
	QueryPerformanceCounter((LARGE_INTEGER*)&startup_time);
	__int64 invertfrequency;
	QueryPerformanceFrequency((LARGE_INTEGER*)&invertfrequency);
	frequency = 1.0f / invertfrequency;
#endif
}

#ifdef LINUX
static inline float timeconverter(const struct timespec & tv) {
	return tv.tv_sec + (float)(tv.tv_nsec) / 1000000000.0f;
#elif defined(EMSCRIPTEN)
static inline float timeconverter(const struct timeval & tv) {
    return (tv.tv_sec + tv.tv_usec / 1000000.0f);
#else
static inline float timeconverter(__int64 tv) {
	return (tv);
#endif
}


#ifdef LINUX
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

float TimeUtil::GetTime() {
    #ifdef LINUX
		struct timespec tv;
		if (clock_gettime(CLOCK_MONOTONIC, &tv) != 0) {
        LOG(FATAL) << "clock_gettime failure";
		}
		sub(tv, startup_time);
    #elif defined(EMSCRIPTEN)
		struct timeval tv;
		gettimeofday(&tv, 0);
		timersub(&tv, &startup_time, &tv);
	#elif defined(WINDOWS)
		__int64 tv;
		QueryPerformanceCounter((LARGE_INTEGER*)&tv);
		tv = (tv - startup_time) * frequency;
    #endif
	return timeconverter(tv);
}

void TimeUtil::Wait(float waitInSeconds) {
   #ifdef LINUX
       float before = GetTime();
       float delta = before - GetTime();
       while (delta < waitInSeconds) {
           struct timespec ts;
           ts.tv_sec = 0;
           ts.tv_nsec = (0.016 - delta) * 1000000000LL;
           nanosleep(&ts, 0);
           delta = before - GetTime();
       }
   #elif defined(WINDOWS)
       LOG(ERROR) << "TODO: TimeUtil::ShouldWaitBeforeNextStep method not implemented for Windows";
   #endif
}
