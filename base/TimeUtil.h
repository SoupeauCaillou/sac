#pragma once

#if SAC_LINUX || SAC_ANDROID
#include <time.h>
#elif SAC_DARWIN || SAC_EMSCRIPTEN
#include <sys/time.h>
#elif SAC_WINDOWS
 #include <Windows.h>
#endif

class TimeUtil
{
	public:
		static void Init(); // plantage de la graine
		static float GetTime();
        static void Wait(float waitInSeconds);

	private:

#if SAC_LINUX || SAC_ANDROID
        static struct timespec startup_time;
#elif SAC_EMSCRIPTEN || SAC_DARWIN
        static struct timeval startup_time;
#elif SAC_WINDOWS
        static __int64 startup_time;
        static double frequency;
#endif
};
