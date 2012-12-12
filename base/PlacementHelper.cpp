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

Vector2 PlacementHelper::GimpPositionToScreen(const Vector2& v) {
    return Vector2(GimpXToScreen(v.X), GimpYToScreen(v.Y));
}
