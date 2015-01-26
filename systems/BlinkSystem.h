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

#if !DISABLE_BLINK_SYSTEM
#include "System.h"

struct BlinkComponent {
    BlinkComponent()
        : enabled(false), visibleDuration(0.0f), hiddenDuration(0.0),
          accum(0.0) {}

    bool enabled;
    float visibleDuration;
    float hiddenDuration;

    float accum;
};

#define theBlinkSystem BlinkSystem::GetInstance()
#if SAC_DEBUG
#define BLINK(e) theBlinkSystem.Get(e, true, __FILE__, __LINE__)
#else
#define BLINK(e) theBlinkSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Blink)
}
;
#endif
