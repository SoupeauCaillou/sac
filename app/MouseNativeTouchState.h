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

#include "base/TouchInputManager.h"
#include <glm/glm.hpp>
#include <list>

// Emulate touch screen with Mouse
class MouseNativeTouchState: public NativeTouchState {
    public:
        MouseNativeTouchState();

        bool isTouching(int index, glm::vec2* windowCoords);

        bool isMoving (int index) ;

        int maxTouchingCount() {
            return 2;
        }

    public:
        //all mouse events which have to be handled by MouseNativeTouchState
        //return 1 if event is handled, 0 else
        int eventSDL(void* event);

        bool _isMoving;
    private:
        //in order : left / right / middle
        bool isButtonDown[3];
        glm::vec2 lastPosition;
#if !ANDROID
    public:
        int wheel;
#endif
};
