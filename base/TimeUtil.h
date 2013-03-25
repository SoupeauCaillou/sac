#pragma once

#ifdef LINUX
	#include <time.h>
#elif defined(WINDOWS)
	#include <Windows.h>
#endif

class TimeUtil
{
	public :
		static void Init(); // plantage de la graine
		static float GetTime();
        static void Wait(float waitInSeconds);

	private:
        #if defined(EMSCRIPTEN) || defined(DARWIN)
		static struct timeval startup_time;
        #elif defined(LINUX)
        static struct timespec startup_time;
		#else
		static __int64 startup_time;
		static double frequency;
        #endif
};
