#include "TouchInputManager.h"
#include "Log.h"
#include "../systems/RenderingSystem.h"
#include "../util/IntersectionUtil.h"
#include "PlacementHelper.h"

TouchInputManager* TouchInputManager::instance = 0;


TouchInputManager* TouchInputManager::Instance() {
	if (instance == 0) instance = new TouchInputManager();
	return instance;
}

void TouchInputManager::init(Vector2 pWorldSize, Vector2 pWindowSize) {
	worldSize = pWorldSize;
	windowSize = pWindowSize;
}

static Vector2 absoluteToCameraPos(const Vector2& pos) {
    /*for (unsigned i=0; i<theRenderingSystem.cameras.size(); i++) {
        const RenderingSystem::Camera& cam = theRenderingSystem.cameras[i];
        if (IntersectionUtil::pointRectangle(pos, cam.screenPosition, cam.screenSize) && cam.enable) {
            return cam.worldPosition + pos * cam.worldSize;
        }
    }*/
    // LOG(WARNING) << "Click outside cameras";
    return pos;
}

void TouchInputManager::Update(float) {
	Vector2 windowPos;

    for (int i=0; i<2; i++) {
    	wasTouching[i] = touching[i];
    	touching[i] = ptr->isTouching(i, &windowPos);
    	if (touching[i]) {
    		lastTouchedScreenPosition[i] = windowToWorld(windowPos, worldSize, windowSize);
            windowPos.X = windowPos.X / (float)PlacementHelper::WindowWidth - 0.5;
            windowPos.Y = 0.5 - windowPos.Y / (float)PlacementHelper::WindowHeight;
            lastTouchedPosition[i] = absoluteToCameraPos(windowPos);
    	}
    }
}

Vector2 TouchInputManager::windowToWorld(const Vector2& windowCoords, const Vector2& worldSize, const Vector2& windowSize) const {
	float x = (windowCoords.X / windowSize.X) * worldSize.X - worldSize.X * 0.5f;
	float y = ((windowSize.Y - windowCoords.Y) / windowSize.Y) * worldSize.Y - worldSize.Y * 0.5f;
	return Vector2(x, y);
}
