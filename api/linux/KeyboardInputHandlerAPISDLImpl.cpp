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



#include "KeyboardInputHandlerAPISDLImpl.h"

#include "systems/TextSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"

#include "base/EntityManager.h"
#include "base/Log.h"

void KeyboardInputHandlerAPISDLImpl::update() {
    std::unique_lock<std::mutex> lock(mutex);

    // each dt, if the key is pressed, call the function
    for (auto it = keyState.begin(); it != keyState.end(); ) {
        std::map<SDL_Keycode, std::function<void()> >* source = 0;

        const SDL_Keycode key = it->first;
        switch (it->second) {
            case KeyState::Pressed:
                source = &keyPressed2callback;
                // advance iterator
                ++it;
                break;
            case KeyState::Releasing:
                source = &keyReleased2callback;
                it->second = KeyState::Released;
                ++it;
                break;
            case KeyState::Released:
                it->second = KeyState::Idle;
                ++it;
                break;
            case KeyState::Idle: {
                // remove it and advance
                auto jt = it;
                it++;
                keyState.erase(jt);
                break;
            }
            default: {
                LOGE("Invalid state: " << it->second << " for key: " << SDL_GetKeyName(it->first));
                ++it;
            }
        }

        // call callback if any
        if (source) {
            auto jt = source->find(key);
            if (jt != source->end()) {
                jt->second();
            }
        }
    }
}

void KeyboardInputHandlerAPISDLImpl::registerToKeyPress(SDL_Keycode value, std::function<void()> f) {
    std::unique_lock<std::mutex> lock(mutex);
    keyPressed2callback[value] = f;
}

void KeyboardInputHandlerAPISDLImpl::registerToKeyRelease(SDL_Keycode value, std::function<void()> f) {
    std::unique_lock<std::mutex> lock(mutex);
    keyReleased2callback[value] = f;
}


bool KeyboardInputHandlerAPISDLImpl::queryKeyState(SDL_Keycode key, KeyState::Enum state) {
    std::unique_lock<std::mutex> lock(mutex);
    auto it = keyState.find(key);
    bool result = (it != keyState.end() && it->second == state);
    return result;
}

int KeyboardInputHandlerAPISDLImpl::eventSDL(const void* inEvent) {
    std::unique_lock<std::mutex> lock(mutex);
    auto event = (SDL_Event*)inEvent;
    if (!event || (event->type != SDL_KEYUP && event->type != SDL_KEYDOWN))
        return 0;

    SDL_Keycode key = event->key.keysym.sym;
    SDL_Scancode scancode = event->key.keysym.scancode;
    #if SAC_DEBUG
    assert(SDL_GetScancodeFromKey(key) == scancode);
    assert(SDL_GetKeyFromScancode(scancode) == key);
    #endif

    std::map<SDL_Keycode, std::function<void ()> > *map;

    KeyState::Enum newState = KeyState::Idle;
    if (event->type == SDL_KEYUP) {
        map = &keyReleased2callback;
        newState = KeyState::Releasing;
    } else if (event->type == SDL_KEYDOWN) {
        map = &keyPressed2callback;
        newState = KeyState::Pressed;
    }

    auto callbackBinding = map->find(key);
    LOGV(1,"key " << ((event->type == SDL_KEYUP) ? "released: " : "pressed: ") <<
        __(scancode) << ", " << __(key) << "): " << ((callbackBinding != map->end()) ? "one" : "no") <<
        " binding found.");
    keyState[key] = newState;
    return (callbackBinding != map->end());
}
