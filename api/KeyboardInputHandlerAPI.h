/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <string>
#include <functional>
#include <map>

class KeyboardInputHandlerAPI {
    public:
    virtual void registerToKeyPress(int key, std::function<void()>) = 0;

    virtual void registerToKeyRelease(int key, std::function<void()>) = 0;

    virtual void update() = 0;

    virtual int eventSDL(const void* event) = 0;

    virtual bool isKeyPressed(int key) = 0;

    virtual bool isKeyReleased(int key) = 0;
};
