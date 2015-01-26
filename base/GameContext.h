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


#include <vector>

/* Enum with all existing API - could be generated from file list: api/*API.h */
namespace ContextAPI {
    enum Enum {
        Ad,
        Asset,
        Communication,
        Exit,
        GameCenter,
        InAppPurchase,
        Joystick,
        KeyboardInputHandler,
        Localize,
        Music,
        Network,
        OpenURL,
        Sound,
        Storage,
        StringInput,
        Vibrate,
        WWW,
        Count
    };
}

template <class T>
ContextAPI::Enum typeToEnum();


#include "GameContext.hpp"

struct GameContext {
    std::vector<APIWrapperI*> wrappers;

    GameContext() { wrappers.resize(ContextAPI::Count); }

    template<class T>
    T* get() {
        APIWrapperI* ptr = wrappers[typeToEnum<T>()];
        if (!ptr) return NULL;
        else return (static_cast<APIWrapper<T>*> (ptr))->as();
    }
};
