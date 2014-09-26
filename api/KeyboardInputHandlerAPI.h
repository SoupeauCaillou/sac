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


namespace KeyboardInputHandler {
    const std::map<std::string, int> keyNameToCodeValue = {
        { "azerty_space", 65 },
        { "azerty_a", 24 },
        { "azerty_b", 56 },
        { "azerty_c", 54 },
        { "azerty_d", 40 },
        { "azerty_e", 26 },
        { "azerty_f", 41 },
        { "azerty_g", 42 },
        { "azerty_h", 43 },
        { "azerty_i", 31 },
        { "azerty_j", 44 },
        { "azerty_k", 45 },
        { "azerty_l", 46 },
        { "azerty_m", 47 },
        { "azerty_n", 57 },
        { "azerty_o", 32 },
        { "azerty_p", 33 },
        { "azerty_q", 38 },
        { "azerty_r", 27 },
        { "azerty_s", 39 },
        { "azerty_t", 28 },
        { "azerty_u", 30 },
        { "azerty_v", 55 },
        { "azerty_w", 52 },
        { "azerty_x", 53 },
        { "azerty_y", 29 },
        { "azerty_z", 25 },
        { "azerty_$", 35 },
        { "azerty_*", 51 },
        { "azerty_enter", 36 },
        { "azerty_<", 94 },
        { "azerty_,", 58 },
        { "azerty_;", 59 },
        { "azerty_:", 60 },
        { "azerty_!", 61 },
        { "azerty_lshift", 62 },

        { "azerty_esc", 9 },

        { "azerty_1", 10 },
        { "azerty_2", 11 },
        { "azerty_3", 12 },
        { "azerty_4", 13 },
        { "azerty_5", 14 },
        { "azerty_6", 15 },
        { "azerty_7", 16 },
        { "azerty_8", 17 },
        { "azerty_9", 18 },
        { "azerty_0", 19 },
        { "azerty_=", 20 },
        { "azerty_%", 21 },
    };

    static inline int k2v(const std::string & key) {
        auto it = keyNameToCodeValue.find(key);
        if (it  != keyNameToCodeValue.end()) {
            return it->second;
        }
        return -1;
    }
}

struct Key {

    enum Type { byPosition = 0, byName } type;
    union {
        int position;
        int keysym;
    };

    inline static Key ByName(int sym) {
        Key k;
        k.type = byName;
        k.keysym = sym;
        return k;
    }
    inline static Key ByPosition(int pos) {
        Key k;
        k.type = byPosition;
        k.position = pos;
        return k;
    }

    bool operator< (const Key & rhs) const {
        if (type == rhs.type) {
            return position < rhs.position;
        } else {
            return type < rhs.type;
        }
    }
};

class KeyboardInputHandlerAPI {
    public:
        virtual void registerToKeyPress(Key key, std::function<void()>) = 0;

        virtual void registerToKeyRelease(Key key, std::function<void()>) = 0;

        virtual void update() = 0;

        virtual int eventSDL(const void* event) = 0;

        virtual bool isKeyPressed(Key key) = 0;

        virtual bool isKeyReleased(Key key) = 0;
};
