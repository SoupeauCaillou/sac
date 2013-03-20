#pragma once

#ifdef LINUX
	#include <time.h>
#elif defined(WINDOWS)
	#include <Windows.h>
#endif

class TimeUtil
{
	public :
		// init the seed
		static void Init(); 
		
		// get the time since the launch of the game
		static float GetTime();

		// sleep if necessary
		static void ShouldWaitBeforeNextStep(float timeBeforeOneStep);

	private:
        #ifdef EMSCRIPTEN
		static struct timeval startup_time;
        #elif defined(LINUX)
        static struct timespec startup_time;
		#elif defined(WINDOWS)
		static __int64 startup_time;
		static float frequency;
        #endif
};
