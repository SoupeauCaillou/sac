#include "TimeUtil.h"
struct timeval TimeUtil::startup_time;

void TimeUtil::init() {
	gettimeofday(&startup_time, 0);
}

float TimeUtil::timeconverter(struct timeval tv) {
	return (tv.tv_sec + tv.tv_usec / 1000000.0f);
}

float TimeUtil::getTime() {
	struct timeval tv;
	gettimeofday(&tv, 0);
	timersub(&tv, &startup_time, &tv);
	return timeconverter(tv);
}
