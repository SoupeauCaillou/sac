/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "PlacementHelper.h"

float PlacementHelper::ScreenWidth = 0;
float PlacementHelper::ScreenHeight = 0;

int PlacementHelper::WindowWidth = 0;
int PlacementHelper::WindowHeight = 0;

float PlacementHelper::GimpWidth = 0;
float PlacementHelper::GimpHeight = 0;

// #define GIMP_WIDTH  800.0f
// #define GIMP_HEIGHT 1280.0f

#define WIDTH_RATIO_TO_SCREEN_WIDTH(r) ((r) * ScreenHeight * GimpWidth/GimpHeight)


float PlacementHelper::GimpWidthToScreen(int width) {
    return WIDTH_RATIO_TO_SCREEN_WIDTH(width / GimpWidth);
}

float PlacementHelper::GimpHeightToScreen(int height) {
    return (ScreenHeight * height) / GimpHeight;
}

float PlacementHelper::GimpYToScreen(int y) {
    return - ScreenHeight * (y - GimpHeight * 0.5) / GimpHeight;
}

float PlacementHelper::GimpXToScreen(int x) {
    return WIDTH_RATIO_TO_SCREEN_WIDTH((x - GimpWidth * 0.5) / GimpWidth);
}
