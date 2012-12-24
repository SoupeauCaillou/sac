#pragma once

#include <sys/time.h>

class TimeUtil
{
	public :
		static void init(); // plantage de la graine
		static float getTime();

	private:
        #ifndef EMSCRIPTEN
		static struct timespec startup_time;
        #else
        static struct timeval startup_time;
        #endif
};
