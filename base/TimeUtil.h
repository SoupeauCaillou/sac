/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#pragma once

#if SAC_LINUX || SAC_ANDROID
#include <time.h>
#elif SAC_DARWIN || SAC_EMSCRIPTEN
#include <sys/time.h>
#elif SAC_WINDOWS
 #include <Windows.h>
#endif

class TimeUtil
{
	public:
		static void Init(); // plantage de la graine
		static float GetTime();
        static void Wait(float waitInSeconds);

	private:

#if SAC_LINUX || SAC_ANDROID
        static struct timespec startup_time;
#elif SAC_EMSCRIPTEN || SAC_DARWIN
        static struct timeval startup_time;
#elif SAC_WINDOWS
        static __int64 startup_time;
        static double frequency;
#endif
};
