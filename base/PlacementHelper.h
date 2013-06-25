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
