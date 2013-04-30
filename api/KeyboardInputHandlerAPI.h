#pragma once

#include <string>
#include <functional>

class KeyboardInputHandlerAPI {
    public:
        enum KeyCode {
            BACKSPACE,
            ENTER,
            SPACE,
            ALPHANUM
        };

        virtual void getUserInput(const int imaxSize = 150)=0;
        virtual bool done(std::string & final)=0;

        virtual void keyRelease(KeyCode, int) {}
        virtual void keyPress(KeyCode, int) {}

        virtual void registerToKeyPress(int value, std::function<void()>) = 0;
};
