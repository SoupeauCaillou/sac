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

void KeyboardInputHandlerAPIGLFWImpl::registerToKeyPress(int value, std::function<void()> f) {
    key2callback[value] = f;
}

int KeyboardInputHandlerAPIGLFWImpl::eventSDL(const SDL_Event* event) {
    int key = event->key.keysym.sym;
    if (event->type == SDL_KEYUP) {
        if (textIsReady)
            return 0;

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
        for (auto it : key2callback) {
            if (it.first == key) {
                it.second();
                return 1;
            }
        }
    }
    return 0;
}
