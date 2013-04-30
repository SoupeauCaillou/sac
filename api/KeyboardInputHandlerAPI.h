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

        virtual void keyPressed(KeyCode, int) {}

        virtual void registerToKey(int value, std::function<void()>) = 0;
};
