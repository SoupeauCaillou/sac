#pragma once

#include "../KeyboardInputHandlerAPI.h"

#include <string>
#include <map>

class KeyboardInputHandlerAPIGLFWImpl : public KeyboardInputHandlerAPI {
    public:
        void registerToKeyPress(int value, std::function<void()> f);

        void update();

        int eventSDL(const void* event);

        bool isKeyPressed(int key);

    private:
        std::map<int, std::pair<bool, std::function<void()>>> key2callback;
};
