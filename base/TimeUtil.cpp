#include "TimeUtil.h"

float TimeUtil::timeconverter(struct timeval tv) {
	return (tv.tv_sec + tv.tv_usec / 1000000.0f);
}

float TimeUtil::getTime(struct timeval *startup_time) {
	struct timeval tv;
	gettimeofday(&tv, 0);
	if (startup_time) timersub(&tv, startup_time, &tv);
	return timeconverter(tv);
}
