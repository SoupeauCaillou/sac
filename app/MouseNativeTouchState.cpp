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

bool MouseNativeTouchState::isTouching(int index, glm::vec2* windowCoords) {
    static bool downLeft = false;
    static bool downRight = false;
    static glm::vec2 position;
    for (auto event : events) {
        int x, y;
        Uint8 buttonType = SDL_GetMouseState(&x, &y);

        switch (index) {
            case 0:
                downLeft = buttonType & SDL_BUTTON(1);
                break;
            case 1:
                downRight = buttonType & SDL_BUTTON(3);
                break;
            default:
                downLeft = downRight = false;
        }

        switch(event.type) {
            //mouse motion
            case SDL_MOUSEMOTION: {
                position.x = x;
                position.y = y;
                break;
            }
            //mouse button clicked
            case SDL_MOUSEBUTTONDOWN: {
                //LOGI("down! index=" << index << " down=" << ((index == 0) ? downLeft : downRight));
                break;
            }
              //mouse button released
            case SDL_MOUSEBUTTONUP: {
                (index == 0) ? downLeft = !downLeft : downRight = ! downRight;
                //LOGI("up! down=" << ((index == 0) ? downLeft : downRight));
                break;
            }
        }
    }
    events.clear();

    *windowCoords = position;
    return (index == 0) ? downLeft : downRight;
}
