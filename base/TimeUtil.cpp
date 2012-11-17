/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "TimeUtil.h"
#include <time.h>
#include "Log.h"

struct timespec TimeUtil::startup_time;

void TimeUtil::init() {
	// gettimeofday(&startup_time, 0);
    clock_gettime(CLOCK_MONOTONIC, &startup_time);
}

static inline float timeconverter(struct timespec tv) {
	return tv.tv_sec + (tv.tv_nsec / 1000000) * 0.001f;
}

static inline void sub(struct timespec& tA, const struct timespec& tB)
{
    if ((tA.tv_nsec - tB.tv_nsec) < 0) {
        tA.tv_sec = tA.tv_sec - tB.tv_sec - 1;
        tA.tv_nsec = 1000000000 + tA.tv_nsec - tB.tv_nsec;
    } else {
        tA.tv_sec = tA.tv_sec - tB.tv_sec;
        tA.tv_nsec = tA.tv_nsec - tB.tv_nsec;
    }
}

float TimeUtil::getTime() {
	struct timespec tv;
	if (clock_gettime(CLOCK_MONOTONIC, &tv) != 0) {
        LOGE("clock_gettime failure");
    }
    // sub(tv, startup_time);
	return timeconverter(tv);
}
