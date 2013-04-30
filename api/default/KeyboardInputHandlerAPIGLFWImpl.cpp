#include "KeyboardInputHandlerAPIGLFWImpl.h"

#include "systems/TextRenderingSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/TransformationSystem.h"

#include "base/EntityManager.h"
#include "base/Log.h"

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

void KeyboardInputHandlerAPIGLFWImpl::keyPress(KeyCode code, int value) {
    //disable the function call
    if (key2callback[value] != 0)
        key2callback[value]();
}

void KeyboardInputHandlerAPIGLFWImpl::keyRelease(KeyCode code, int value) {
    if (textIsReady)
        return;

    switch (code) {
        case BACKSPACE:
            //remove last char
            if (currentText.size() > 0) {
                currentText.erase(currentText.end() - 1);
            }
            break;
        case ENTER:
            if (currentText.size() > 0) {
                textIsReady = true;
            }
            break;
        case SPACE:
        case ALPHANUM:
            if ((int)currentText.size() < maxSize) {
                currentText.append(1u, (char)value);
            }
            break;
    }
    LOGI("current text: " << currentText);
}

void KeyboardInputHandlerAPIGLFWImpl::registerToKeyPress(int value, std::function<void()> f) {
    key2callback[value] = f;
}
