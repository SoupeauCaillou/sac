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
