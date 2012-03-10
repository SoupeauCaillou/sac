#pragma once

#include <sys/time.h>

class TimeUtil
{
	public :
		static float timeconverter(struct timeval tv);
		static float getTime(struct timeval *startup_time);
};
