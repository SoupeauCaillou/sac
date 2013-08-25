#include "KeyboardInputHandlerAPISDLImpl.h"

#include "systems/TextSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"

#include "base/EntityManager.h"
#include "base/Log.h"

#include <SDL/SDL.h>

void KeyboardInputHandlerAPIGLFWImpl::update() {
    //each dt, if the key is pressed, call the function
    for (auto it : key2callback) {
        if (it.second.first) {
            if (it.second.second != 0) {
                it.second.second();
            }
        }
    }
}

void KeyboardInputHandlerAPIGLFWImpl::registerToKeyPress(int value, std::function<void()> f) {
    key2callback[value] = std::pair<bool, std::function<void()>> (false, f);
}

bool KeyboardInputHandlerAPIGLFWImpl::isKeyPressed(int key) {
    return (key2callback[key].first == true);
}


int KeyboardInputHandlerAPIGLFWImpl::eventSDL(const void* inEvent) {
    auto event = (SDL_Event*)inEvent;
    if (!event || (event->type != SDL_KEYUP && event->type != SDL_KEYDOWN))
        return 0;

    int scancode = event->key.keysym.scancode;
    auto unicode = (char)event->key.keysym.unicode;

    if (event->type == SDL_KEYUP) {
        LOGV(2, "key released, scancode: " << scancode);
        for (auto & it : key2callback) {
            if (it.first == scancode) {
                it.second.first = false;
                return 1;
            }
        }
        return 0;
    } else if (event->type == SDL_KEYDOWN) {
        LOGV(2, "key pressed, scancode: " << scancode << " unicode value: " << unicode);
        for (auto & it : key2callback) {
            if (it.first == scancode) {
                it.second.first = true;
                return 1;
            }
        }
    }
    return 0;
}
