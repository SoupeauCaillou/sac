#pragma once

#ifdef LINUX
	#include <time.h>
#else
	#include <Windows.h>
#endif

class TimeUtil
{
	public :
		static void init(); // plantage de la graine
		static float getTime();

	private:
        #ifdef EMSCRIPTEN
		static struct timeval startup_time;
        #elif defined(LINUX)
        static struct timespec startup_time;
		#else
		static __int64 startup_time;
		static float frequency;
        #endif
};
