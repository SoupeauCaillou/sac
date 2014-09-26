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

#include <SDL.h>

void KeyboardInputHandlerAPIGLFWImpl::update() {
    std::unique_lock<std::mutex> lock(mutex);

    //each dt, if the key is pressed, call the function
    for (auto it = keyState.begin(); it != keyState.end(); ) {
        std::map<Key, std::function<void()> >* source = 0;

        const Key key = it->first;
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
                LOGE("Invalid state: " << it->second << " for key: " << it->first.keysym);
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

void KeyboardInputHandlerAPIGLFWImpl::registerToKeyPress(Key value, std::function<void()> f) {
    std::unique_lock<std::mutex> lock(mutex);
    keyPressed2callback[value] = f;
}

void KeyboardInputHandlerAPIGLFWImpl::registerToKeyRelease(Key value, std::function<void()> f) {
    std::unique_lock<std::mutex> lock(mutex);
    keyReleased2callback[value] = f;
}


bool KeyboardInputHandlerAPIGLFWImpl::queryKeyState(Key key, KeyState::Enum state) {
    std::unique_lock<std::mutex> lock(mutex);
    auto it = keyState.find(key);
    bool result = (it != keyState.end() && it->second == state);
    LOGI_IF(it != keyState.end(), key.keysym << " -> " << result << __(it->second));
    return result;
}

int KeyboardInputHandlerAPIGLFWImpl::eventSDL(const void* inEvent) {
    std::unique_lock<std::mutex> lock(mutex);
    auto event = (SDL_Event*)inEvent;
    if (!event || (event->type != SDL_KEYUP && event->type != SDL_KEYDOWN))
        return 0;

    int scancode = event->key.keysym.scancode;
    int sym = event->key.keysym.sym;
    Key byPosition = Key::ByPosition(scancode);
    Key byName = Key::ByName(sym);

    if (event->type == SDL_KEYUP) {
        LOGV(1, "key released (" << __(scancode) << ", " << __(sym) << ")");
        keyState[byPosition] = KeyState::Releasing;
        keyState[byName] = KeyState::Releasing;
        const auto& p1 = keyReleased2callback.find(byPosition);
        const auto& p2 = keyReleased2callback.find(byName);
        return (p1 != keyReleased2callback.end() || p2 != keyReleased2callback.end());
    } else if (event->type == SDL_KEYDOWN) {
        LOG_USAGE_ONLY(auto unicode = event->key.keysym.unicode);
        LOGV(1, "key pressed (" << __(scancode) << ", " << __(sym) << ", " << __(unicode) << ")");
        keyState[byPosition] = KeyState::Pressed;
        keyState[byName] = KeyState::Pressed;
        const auto& p1 = keyPressed2callback.find(byPosition);
        const auto& p2 = keyPressed2callback.find(byName);
        return (p1 != keyPressed2callback.end() || p2 != keyPressed2callback.end());
    }
    return 0;
}
