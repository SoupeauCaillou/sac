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

#if SAC_WINDOWS
typedef unsigned char uint8_t;
typedef unsigned __int32 uint32_t;
#else
#include <cstdint>
#endif

#if SAC_WINDOWS
#define PRAGMA_WARNING(x) __pragma(x)
#else
#define PRAGMA_WARNING(x)
#endif

typedef uint8_t bitfield8_t;
typedef uint16_t bitfield16_t;
typedef uint32_t bitfield32_t;
typedef bitfield32_t bitfield_t;
