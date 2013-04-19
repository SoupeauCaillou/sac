#pragma once

#include "../KeyboardInputHandlerAPI.h"

#include <string>

class KeyboardInputHandlerAPIGLFWImpl : public KeyboardInputHandlerAPI {
    public:
        KeyboardInputHandlerAPIGLFWImpl();

        void getUserInput(const int imaxSize);
        bool done(std::string & final);

        void keyPressed(KeyCode code, int value);

    private:
        bool textIsReady;
        std::string currentText;
        int maxSize;
};
