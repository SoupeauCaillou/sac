#pragma once

#ifdef SAC_LINUX
	#include <time.h>
#elif defined(SAC_WINDOWS)
	#include <Windows.h>
#endif

class TimeUtil
{
	public :
		static void Init(); // plantage de la graine
		static float GetTime();
        static void Wait(float waitInSeconds);

	private:
        #if defined(SAC_EMSCRIPTEN) || defined(SAC_DARWIN)
		static struct timeval startup_time;
        #elif defined(SAC_LINUX)
        static struct timespec startup_time;
		#else
		static __int64 startup_time;
		static double frequency;
        #endif
};
