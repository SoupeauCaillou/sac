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
    for (auto it = keyState.begin(); it != keyState.end(); ) {
        std::map<int, std::function<void()> >* source = 0;

        const int scancode = it->first;
        switch (it->second) {
            case KeyState::Pressed:
                source = &keyPressed2callback;
                // advance iterator
                ++it;
                break;
            case KeyState::Released:
                source = &keyReleased2callback;
                // remove it and advance
                keyState.erase(it++);
                break;
        }

        // call callback if any
        auto jt = source->find(scancode);
        if (jt != source->end()) {
            jt->second();
        }
    }
}

void KeyboardInputHandlerAPIGLFWImpl::registerToKeyPress(int value, std::function<void()> f) {
    keyPressed2callback[value] = f;
}

void KeyboardInputHandlerAPIGLFWImpl::registerToKeyRelease(int value, std::function<void()> f) {
    keyReleased2callback[value] = f;
}


bool KeyboardInputHandlerAPIGLFWImpl::isKeyPressed(int key) {
    auto it = keyState.find(key);
    return (it != keyState.end() && it->second == KeyState::Pressed);
}


int KeyboardInputHandlerAPIGLFWImpl::eventSDL(const void* inEvent) {
    auto event = (SDL_Event*)inEvent;
    if (!event || (event->type != SDL_KEYUP && event->type != SDL_KEYDOWN))
        return 0;

    int scancode = event->key.keysym.scancode;
    auto unicode = (char)event->key.keysym.unicode;

    if (event->type == SDL_KEYUP) {
        LOGV(2, "key released, scancode: " << scancode);
        keyState[scancode] = KeyState::Released;
        const auto& p = keyReleased2callback.find(scancode);
        return (p != keyReleased2callback.end());
    } else if (event->type == SDL_KEYDOWN) {
        LOGV(2, "key pressed, scancode: " << scancode << " unicode value: " << unicode);
        keyState[scancode] = KeyState::Pressed;
        const auto& p = keyPressed2callback.find(scancode);
        return (p != keyPressed2callback.end());
    }
    return 0;
}
