/*
    This file is part of sac.

    @author Soupe au Caillou

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
#pragma once

#include "base/TouchInputManager.h"
#include <glm/glm.hpp>

// Emulate touch screen with Mouse
class MouseNativeTouchState: public NativeTouchState {
    public:
        bool isTouching(int, glm::vec2* windowCoords) const;

        int maxTouchingCount() {
            return 2;
        }
};
