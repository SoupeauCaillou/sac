#include "KeyboardInputHandlerAPISDLImpl.h"

#include "systems/TextRenderingSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"

#include "base/EntityManager.h"
#include "base/Log.h"

#include <SDL/SDL.h>

KeyboardInputHandlerAPIGLFWImpl::KeyboardInputHandlerAPIGLFWImpl() : textIsReady(true) {}

void KeyboardInputHandlerAPIGLFWImpl::askUserInput(const std::string& initialText, const int imaxSize) {
    LOGI("Please enter something...");
    currentText = initialText;
    textIsReady = false;
    maxSize = imaxSize;
}

void KeyboardInputHandlerAPIGLFWImpl::cancelUserInput() {
    textIsReady = true;
    currentText.clear();
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
        LOGV(2, "key pressed: " << key << " unicode value: " << event->key.keysym.unicode);

        if (! textIsReady) {
            //remove last char
            if (key == SDLK_BACKSPACE) {
                if (currentText.size() > 0) {
                    currentText.erase(currentText.end() - 1);
                }
            //finish the input
            } else if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
                if (currentText.size() > 0) {
                    textIsReady = true;
                }
            //check the character is valid
            } else if ((unicode == ' ') || isalnum(unicode)) {
                if ((int)currentText.size() < maxSize) {
                    currentText.append(1u, unicode);
                }
            } else {
                return 0;
            }

            LOGI("current text: " << currentText);
            return 1;
        } else {
            for (auto & it : key2callback) {
                if (it.first == key) {
                    it.second.first = true;
                    return 1;
                }
            }
        }
    }
    return 0;
}
