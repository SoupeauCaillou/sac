#include "PlacementHelper.h"

glm::vec2 PlacementHelper::ScreenSize(0.f);

glm::vec2 PlacementHelper::WindowSize(0);

glm::vec2 PlacementHelper::GimpSize(0.f);

#define REF_RATIO (GimpSize.x/GimpSize.y)

#define HEIGHT_IS_REF (REF_RATIO < 1)
//(PlacementHelper::ScreenSize.y / PlacementHelper::ScreenSize.x <= REF_RATIO)

#define WIDTH_RATIO_TO_SCREEN_WIDTH(r) ((r) * ScreenSize.y * GimpSize.x/GimpSize.y)
#define HEIGHT_RATIO_TO_SCREEN_HEIGHT(r) ((r) * ScreenSize.x * GimpSize.y/GimpSize.x)


float PlacementHelper::GimpWidthToScreen(int width) {
	if (HEIGHT_IS_REF) {
    	return WIDTH_RATIO_TO_SCREEN_WIDTH(width / GimpSize.x);
    } else {
    	return (ScreenSize.x * width) / GimpSize.x;
    }
}

float PlacementHelper::GimpHeightToScreen(int height) {
	if (HEIGHT_IS_REF) {
    	return (ScreenSize.y * height) / GimpSize.y;
    } else {
    	return HEIGHT_RATIO_TO_SCREEN_HEIGHT(height / GimpSize.y);
    }
}

float PlacementHelper::GimpYToScreen(int y) {
	if (HEIGHT_IS_REF) {
    	return - ScreenSize.y * (y - GimpSize.y * 0.5f) / GimpSize.y;
    } else {
    	return - HEIGHT_RATIO_TO_SCREEN_HEIGHT((y - GimpSize.y * 0.5f) / GimpSize.y);
    }
}

float PlacementHelper::GimpXToScreen(int x) {
	if (HEIGHT_IS_REF) {
    	return WIDTH_RATIO_TO_SCREEN_WIDTH((x - GimpSize.x * 0.5f) / GimpSize.x);
    } else {
    	return (ScreenSize.x * (x - GimpSize.x * 0.5f)) / GimpSize.x;
    }
}

glm::vec2 PlacementHelper::GimpSizeToScreen(const glm::vec2& v) {
 	return glm::vec2(GimpWidthToScreen(v.x), GimpHeightToScreen(v.y));
}

glm::vec2 PlacementHelper::GimpPositionToScreen(const glm::vec2& v) {
    return glm::vec2(GimpXToScreen(v.x), GimpYToScreen(v.y));
}
