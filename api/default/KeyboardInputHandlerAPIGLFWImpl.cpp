#include "KeyboardInputHandlerAPIGLFWImpl.h"

#include "systems/TextRenderingSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"

#include "base/EntityManager.h"
#include "base/Log.h"

#include <SDL/SDL.h>

KeyboardInputHandlerAPIGLFWImpl::KeyboardInputHandlerAPIGLFWImpl() : textIsReady(true) {}

void KeyboardInputHandlerAPIGLFWImpl::getUserInput(const int imaxSize) {
    LOGI("Please enter something...");
    currentText.clear();
    textIsReady = false;
    maxSize = imaxSize;
}

bool KeyboardInputHandlerAPIGLFWImpl::done(std::string & final) {
    final = currentText;
    if (textIsReady && currentText.size() > 0) {
	    return true;
    }
    return false;
}

void KeyboardInputHandlerAPIGLFWImpl::update() {
    for (auto it : key2callback) {
        if (it.second.first) {
            it.second.second();
        }
    }
}

void KeyboardInputHandlerAPIGLFWImpl::registerToKeyPressPerScancode(int value, std::function<void()> f) {
    key2callback[value] = std::pair<bool, std::function<void()>> (false, f);
}

int KeyboardInputHandlerAPIGLFWImpl::eventSDL(const SDL_Event* event) {
    int key = event->key.keysym.sym;
    int scancode = event->key.keysym.scancode;
    if (event->type == SDL_KEYUP) {
        //if we don't want some text, then check the map, maybe the key was registered
        if (textIsReady) {
            //unfortunately auto doesn't work here (maybe does it create a const iterator ? anyway, with auto, that won't modify the key at all :(
            //for (auto it : key2callback) {
            std::map<int, std::pair<bool, std::function<void()>>>::iterator it;
            for (it = key2callback.begin(); it != key2callback.end(); ++it) {
                if (it->first == scancode) {
                    it->second.first = false;
                    return 1;
                }
            }
            return 0;
        }
        if (key == SDLK_BACKSPACE) {
            //remove last char
            if (currentText.size() > 0) {
                currentText.erase(currentText.end() - 1);
            }
        } else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
            if (currentText.size() > 0) {
                textIsReady = true;
            }
        } else if ((key == ' ') || isalnum(key)) {
            if ((int)currentText.size() < maxSize) {
                currentText.append(1u, key);
            }
        } else {
            return 0;
        }

        LOGI("current text: " << currentText);
        return 1;

    } else if (event->type == SDL_KEYDOWN) {
        //unfortunately auto doesn't work here (maybe does it create a const iterator ? anyway, with auto, that won't modify the key at all :(
        //for (auto it : key2callback) {
        std::map<int, std::pair<bool, std::function<void()>>>::iterator it;
        for (it = key2callback.begin(); it != key2callback.end(); ++it) {
            if (it->first == scancode) {
                it->second.first = true;
                return 1;
            }
        }
    }
    return 0;
}
