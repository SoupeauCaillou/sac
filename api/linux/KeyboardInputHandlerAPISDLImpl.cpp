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
    int key = event->key.keysym.sym;
    auto unicode = (char)event->key.keysym.unicode;

    if (event->type == SDL_KEYUP) {
        // LOGI("key released: " << key);
        for (auto & it : key2callback) {
            if (it.first == key) {
                it.second.first = false;
                return 1;
            }
        }
        return 0;
    } else if (event->type == SDL_KEYDOWN) {
        LOGV(2, "key pressed: " << key << " unicode value: " << unicode);
        for (auto & it : key2callback) {
            if (it.first == key) {
                it.second.first = true;
                return 1;
            }
        }
    }
    return 0;
}
