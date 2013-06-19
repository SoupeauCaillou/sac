#pragma once

#include <string>
#include <functional>

class KeyboardInputHandlerAPI {
    public:
        virtual void askUserInput(const std::string& initial = "", const int imaxSize = 150) = 0;
        virtual void cancelUserInput() = 0;

        //set final string to the current input state and return true if user pressed 'enter'
        virtual bool done(std::string & final) = 0;

        virtual void registerToKeyPress(int key, std::function<void()>) = 0;

        virtual void update() = 0;

        virtual int eventSDL(const void* event) = 0;

        virtual bool isKeyPressed(int key) = 0;
};
