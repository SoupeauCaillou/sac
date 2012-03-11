#pragma once

#include <sys/time.h>

class TimeUtil
{
	public :
		static void init(); // plantage de la graine
		static float timeconverter(struct timeval tv);
		static float getTime();
		
	private:
		static struct timeval startup_time;
};
