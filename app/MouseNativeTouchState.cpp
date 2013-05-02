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
#include "MouseNativeTouchState.h"

#if SAC_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#include <base/Log.h>

MouseNativeTouchState::MouseNativeTouchState() {
    isButtonDown[0] =
    isButtonDown[1] =
    isButtonDown[2] = false;
}

bool MouseNativeTouchState::isTouching(int index, glm::vec2* windowCoords) {
    *windowCoords = lastPosition;
    return isButtonDown[index];
}

int MouseNativeTouchState::eventSDL(SDL_Event* event) {
    bool isDownEvent;

    lastPosition.x = event->motion.x;
    lastPosition.y = event->motion.y;

    switch(event->type) {
        case SDL_MOUSEMOTION: {
            return 1;
        }
        //mouse button clicked
        case SDL_MOUSEBUTTONDOWN: {
            isDownEvent = true;

            break;
        }
          //mouse button released
        case SDL_MOUSEBUTTONUP: {
            isDownEvent = false;
            break;
        }

        //unrecognized event, dont handle it
        default:
            return 0;
    }


    switch (event->button.button) {
        //left btn
        case 1:
            isButtonDown[0] = isDownEvent;
            break;
        case 2:
            isButtonDown[2] = isDownEvent;
            break;
        case 3:
            isButtonDown[1] = isDownEvent;
            break;
    }
    //LOGI("/!\\SDL ID " << (int)event->button.button << " is " << (isDownEvent ? "down!" : "up!"));
    return 1;
}
