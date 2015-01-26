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

struct PlacementHelper {
    static glm::vec2 ScreenSize;
    static glm::vec2 WindowSize;
    static glm::vec2 GimpSize;

    static float GimpWidthToScreen(int width);
    static float GimpHeightToScreen(int height);
    static glm::vec2 GimpSizeToScreen(const glm::vec2& v);

    static float GimpYToScreen(int y);
    static float GimpXToScreen(int x);
    static glm::vec2 GimpPositionToScreen(const glm::vec2& v);
};
