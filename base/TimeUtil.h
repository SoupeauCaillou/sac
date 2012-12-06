#pragma once

#include <sys/time.h>

class TimeUtil
{
	public :
		static void init(); // plantage de la graine
		static float getTime();

	private:
		static struct timespec startup_time;
};
