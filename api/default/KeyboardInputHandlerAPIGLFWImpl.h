#pragma once

#include "../KeyboardInputHandlerAPI.h"

#include <string>
#include <map>

class KeyboardInputHandlerAPIGLFWImpl : public KeyboardInputHandlerAPI {
    public:
        KeyboardInputHandlerAPIGLFWImpl();

        void getUserInput(const int imaxSize);
        bool done(std::string & final);

        void registerToKeyPress(int value, std::function<void()> f);


        int eventSDL(const SDL_Event* event);

    private:
        bool textIsReady;
        std::string currentText;
        int maxSize;

        std::map<int, std::function<void()>> key2callback;
};
