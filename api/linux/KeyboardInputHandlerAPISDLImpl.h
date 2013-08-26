#pragma once

#include "../KeyboardInputHandlerAPI.h"

#include <string>
#include <map>

namespace KeyState {
	enum Enum {
		Pressed,
		Released
	};
}

class KeyboardInputHandlerAPIGLFWImpl : public KeyboardInputHandlerAPI {
    public:
        void registerToKeyPress(int value, std::function<void()> f);

        void registerToKeyRelease(int value, std::function<void()> f);

        void update();

        int eventSDL(const void* event);

        bool isKeyPressed(int key);

    private:
        std::map<int, KeyState::Enum> keyState;
        std::map<int, std::function<void()> > keyPressed2callback;
        std::map<int, std::function<void()> > keyReleased2callback;
};
