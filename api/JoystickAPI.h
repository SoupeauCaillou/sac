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

#include <glm/glm.hpp>

class JoystickAPI {
    public:
        virtual bool hasClicked(int idx, int btn) const = 0;
        virtual bool hasDoubleClicked(int idx, int btn) const = 0;
        virtual void resetDoubleClick(int idx, int btn) = 0;
        virtual const glm::vec2& getPadDirection(int idx, int pad) const = 0;
        virtual void update(float dt) = 0;
        virtual int eventSDL(void* event) = 0;
};
