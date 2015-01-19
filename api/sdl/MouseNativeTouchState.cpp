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



#include "MouseNativeTouchState.h"

#if SAC_EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

MouseNativeTouchState::MouseNativeTouchState() {
    isButtonDown[0] =
    isButtonDown[1] =
    isButtonDown[2] = false;
}

bool MouseNativeTouchState::isTouching(int index, glm::vec2* windowCoords) {
    std::unique_lock<std::mutex> lock(mutex);
    *windowCoords = lastPosition;
    return isButtonDown[index];
}

#if SAC_ANDROID
int MouseNativeTouchState::eventSDL(SDL_Event* event) {
    return 0;
}
#else
#include <SDL.h>
int MouseNativeTouchState::eventSDL(void* inEvent) {
    std::unique_lock<std::mutex> lock(mutex);
    auto event = (SDL_Event*)inEvent;

    bool isDownEvent;


    switch(event->type) {
        case SDL_WINDOWEVENT:
            if (event->window.event == SDL_WINDOWEVENT_LEAVE) {
                // app lose focus == mouse up
                isDownEvent = false;
            }
            break;
        case SDL_MOUSEMOTION: {
            lastPosition.x = event->motion.x;
            lastPosition.y = event->motion.y;
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
        case SDL_MOUSEWHEEL: {
            wheel = event->wheel.y;
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
#if !ANDROID
        // case SDL_BUTTON_WHEELUP:
        //     wheel = 1;
        //     break;
        // case SDL_BUTTON_WHEELDOWN:
        //     wheel = -1;
        //     break;
#endif
    }

    //LOGI("/!\\SDL ID " << (int)event->button.button << " is " << (isDownEvent ? "down!" : "up!"));
    return 1;
}
#endif
