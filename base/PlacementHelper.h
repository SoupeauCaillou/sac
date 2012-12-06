#pragma once

#include "Vector2.h"

struct PlacementHelper {
    static float ScreenWidth, ScreenHeight;
    static int WindowWidth, WindowHeight;
    static float GimpWidth, GimpHeight;

    static float GimpWidthToScreen(int width);
    static float GimpHeightToScreen(int height);
    static Vector2 GimpSizeToScreen(const Vector2& v);

    static float GimpYToScreen(int y);
    static float GimpXToScreen(int x);
};
