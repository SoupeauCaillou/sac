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
#include <SDL/SDL.h>
#include <emscripten/emscripten.h>
#else
#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glfw.h>
#endif

#include <base/Log.h>

bool MouseNativeTouchState::isTouching(int index, glm::vec2* windowCoords) const {
#if SAC_EMSCRIPTEN
     static bool down = false;
     static glm::vec2 position;
      SDL_Event event;
      while (SDL_PollEvent(&event)) {
        switch(event.type) {

        //mouse motion
        case SDL_MOUSEMOTION: {
            SDL_MouseMotionEvent *m = (SDL_MouseMotionEvent*)&event;
            int x,y;
            SDL_GetMouseState(&x, &y);
            position.x = x;
            position.y = y;
            break;
          }
          //mouse button clicked
          case SDL_MOUSEBUTTONDOWN: {
            // SDL_GetMouseState(&x, &y);
            SDL_MouseButtonEvent *m = (SDL_MouseButtonEvent*)&event;
            if (m->button == SDL_BUTTON_LEFT) {
                // windowCoords->X = m->x;
                // windowCoords->Y = m->y;
                down = true;
                LOGI("Mouse down " << windowCoords->x << ", " << windowCoords->y)
            }
            break;
          }
          //mouse button released
          case SDL_MOUSEBUTTONUP: {
            SDL_MouseButtonEvent *m = (SDL_MouseButtonEvent*)&event;
            if (m->button == SDL_BUTTON_LEFT) {
                down = false;
                LOGI("Mouse up");
            }
            break;
          }
          //key pressed
          case SDL_KEYDOWN: {
            /*
            if (globalFTW == 0)
                break;

            if (TEXT_RENDERING(globalFTW)->show) {
                char c;
                switch (event.key.keysym.sym) {
                    case SDLK_BACKSPACE:
                        if (TEXT_RENDERING(nameInput->nameEdit)->show) {
                            std::string& text = TEXT_RENDERING(nameInput->nameEdit)->text;
                            if (text.length() > 0) {
                                text.resize(text.length() - 1);
                            }
                        }
                        break;
                    case SDLK_RETURN:
                        if (TEXT_RENDERING(nameInput->nameEdit)->show) {
                            nameInput->textIsReady = true;
                        }
                        break;
                    default:
                        c = event.key.keysym.sym;
                }

                if (isalnum(c) || c == ' ') {
                    if (TEXT_RENDERING(globalFTW)->text.length() > 10)
                        break;
                    // filter out all unsupported keystrokes
                    TEXT_RENDERING(globalFTW)->text.push_back((char)c);
                }
            }*/
            break;
          }
        }
       }
       *windowCoords = position;
    return down;
#else
    int x,y;
    glfwGetMousePos(&x, &y);
    windowCoords->x = x;
    windowCoords->y = y;
    switch (index) {
        case 0:
            return glfwGetMouseButton(GLFW_MOUSE_BUTTON_1) == GLFW_PRESS;
        case 1:
            return glfwGetMouseButton(GLFW_MOUSE_BUTTON_2) == GLFW_PRESS;
        default:
            return false;
    }

#endif
}
