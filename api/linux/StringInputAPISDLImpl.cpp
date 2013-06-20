#include "StringInputAPISDLImpl.h"

#include "base/Log.h"

#include "SDL/SDL.h"

void StringInputAPISDLImpl::askUserInput(const std::string& initialText, const int imaxSize) {
    LOGI("Please enter something...");
    currentText = initialText;
    textIsReady = false;
    maxSize = imaxSize;
}

void StringInputAPISDLImpl::cancelUserInput() {
    textIsReady = true;
    currentText.clear();
}

bool StringInputAPISDLImpl::done(std::string & final) {
    final = currentText;
    if (textIsReady && currentText.size() > 0) {
        return true;
    }
    return false;
}

int StringInputAPISDLImpl::eventSDL(const void* inEvent) {
    if (textIsReady) {
        return 0;
    }

    auto event = (SDL_Event*)inEvent;
    int key = event->key.keysym.sym;
    auto unicode = (char)event->key.keysym.unicode;

    if (event->type == SDL_KEYDOWN) {
        LOGV(2, "key pressed: " << key << " unicode value: " << event->key.keysym.unicode);

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
    }

    return 0;
}
