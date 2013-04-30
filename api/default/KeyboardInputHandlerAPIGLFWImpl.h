#pragma once

#include "../KeyboardInputHandlerAPI.h"

#include <string>
#include <map>

class KeyboardInputHandlerAPIGLFWImpl : public KeyboardInputHandlerAPI {
    public:
        KeyboardInputHandlerAPIGLFWImpl();

        void getUserInput(const int imaxSize);
        bool done(std::string & final);

        void keyRelease(KeyCode code, int value);

        void checkKeyPress();
        void registerToKeyPress(int value, std::function<void()> f);

    private:
        bool textIsReady;
        std::string currentText;
        int maxSize;

        std::map<int, std::function<void()>> key2callback;
};
