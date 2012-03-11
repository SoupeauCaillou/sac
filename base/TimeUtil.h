#pragma once

#include <sys/time.h>

class TimeUtil
{
	public :
		static void init();
		static float timeconverter(struct timeval tv);
		static float getTime();
		
	private:
		static struct timeval startup_time;
};
