#pragma once

#include <string>
#include <functional>

#include <SDL/SDL.h>

class KeyboardInputHandlerAPI {
    public:
        virtual void getUserInput(const int imaxSize = 150)=0;
        virtual bool done(std::string & final)=0;

        virtual void registerToKeyPress(int value, std::function<void()>) = 0;

        virtual int eventSDL(const SDL_Event* event) = 0;
};
