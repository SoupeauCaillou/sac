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

#include "base/Log.h"
#include <cstring>

class MurmurHash {
    public:
#if SAC_DEBUG
        static inline unsigned staticHash(const char *s, unsigned value) {
            LOGF_IF(compute(s, strlen(s), 0) != value, 	
				"Invalid precalculated hash value. Expected: " << compute(s, strlen(s), 0) << 
				". Actual: " << value);
            return value;
        }
#else
        #define staticHash(s,v) (v)
#endif

        static unsigned int compute( const void * key, int len, unsigned int seed = 0x12345678);
};
