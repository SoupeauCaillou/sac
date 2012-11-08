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

#define REF_RATIO (GimpWidth/GimpHeight)

#define HEIGHT_IS_REF (REF_RATIO < 1)
//(PlacementHelper::ScreenHeight / PlacementHelper::ScreenWidth <= REF_RATIO)

#define WIDTH_RATIO_TO_SCREEN_WIDTH(r) ((r) * ScreenHeight * GimpWidth/GimpHeight)
#define HEIGHT_RATIO_TO_SCREEN_HEIGHT(r) ((r) * ScreenWidth * GimpHeight/GimpWidth)


float PlacementHelper::GimpWidthToScreen(int width) {
	if (HEIGHT_IS_REF) {
    	return WIDTH_RATIO_TO_SCREEN_WIDTH(width / GimpWidth);
    } else {
    	return (ScreenWidth * width) / GimpWidth;
    }
}

float PlacementHelper::GimpHeightToScreen(int height) {
	if (HEIGHT_IS_REF) {
    	return (ScreenHeight * height) / GimpHeight;
    } else {
    	return HEIGHT_RATIO_TO_SCREEN_HEIGHT(height / GimpHeight);
    }
}

float PlacementHelper::GimpYToScreen(int y) {
	if (HEIGHT_IS_REF) {
    	return - ScreenHeight * (y - GimpHeight * 0.5) / GimpHeight;
    } else {
    	return - HEIGHT_RATIO_TO_SCREEN_HEIGHT((y - GimpHeight * 0.5) / GimpHeight);
    }
}

float PlacementHelper::GimpXToScreen(int x) {
	if (HEIGHT_IS_REF) {
    	return WIDTH_RATIO_TO_SCREEN_WIDTH((x - GimpWidth * 0.5) / GimpWidth);
    } else {
    	return (ScreenWidth * (x - GimpWidth * 0.5)) / GimpWidth;
    }
}

Vector2 PlacementHelper::GimpSizeToScreen(const Vector2& v) {
 	return Vector2(GimpWidthToScreen(v.X), GimpHeightToScreen(v.Y));
}
