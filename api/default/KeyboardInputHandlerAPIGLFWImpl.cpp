#include "KeyboardInputHandlerAPIGLFWImpl.h"

#include "systems/TextRenderingSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"

#include "base/EntityManager.h"
#include "base/Log.h"

#include <SDL/SDL.h>

KeyboardInputHandlerAPIGLFWImpl::KeyboardInputHandlerAPIGLFWImpl() : textIsReady(true) {}

void KeyboardInputHandlerAPIGLFWImpl::getUserInput(const std::string& initialText, const int imaxSize) {
    LOGI("Please enter something...");
    currentText = initialText;
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


int KeyboardInputHandlerAPIGLFWImpl::eventSDL(const SDL_Event* event) {
    int key = event->key.keysym.sym;
    if (event->type == SDL_KEYUP) {
        //if we don't want some text, then check the map, maybe the key was registered
        if (textIsReady) {
            // LOGI("key released: " << key);
            //unfortunately auto doesn't work here (maybe does it create a const iterator ? anyway, with auto, that won't modify the key at all :(
            //for (auto it : key2callback) {
            std::map<int, std::pair<bool, std::function<void()>>>::iterator it;
            for (it = key2callback.begin(); it != key2callback.end(); ++it) {
                if (it->first == key) {
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
        // LOGI("key pressed: " << key);
        //unfortunately auto doesn't work here (maybe does it create a const iterator ? anyway, with auto, that won't modify the key at all :(
        //for (auto it : key2callback) {
        std::map<int, std::pair<bool, std::function<void()>>>::iterator it;
        for (it = key2callback.begin(); it != key2callback.end(); ++it) {
            if (it->first == key) {
                it->second.first = true;
                return 1;
            }
        }
    }
    return 0;
}
