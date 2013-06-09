#pragma once

#include "../KeyboardInputHandlerAPI.h"

#include <string>
#include <map>

class KeyboardInputHandlerAPIGLFWImpl : public KeyboardInputHandlerAPI {
    public:
        KeyboardInputHandlerAPIGLFWImpl();

        void askUserInput(const std::string& initial, const int imaxSize);
        void cancelUserInput();
        bool done(std::string & final);

        void registerToKeyPress(int value, std::function<void()> f);

        void update();

        int eventSDL(const SDL_Event* event);

        bool isKeyPressed(int key);

    private:
        bool textIsReady;
        std::string currentText;
        int maxSize;

        std::map<int, std::pair<bool, std::function<void()>>> key2callback;
};
