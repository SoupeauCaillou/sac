#pragma once

#include <string>
#include <functional>

class KeyboardInputHandlerAPI {
    public:
        virtual void registerToKeyPress(int key, std::function<void()>) = 0;

        virtual void update() = 0;

        virtual int eventSDL(const void* event) = 0;

        virtual bool isKeyPressed(int key) = 0;
};
