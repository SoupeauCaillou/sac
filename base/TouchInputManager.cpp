#include "TouchInputManager.h"
#include <glog/logging.h>
#include "../systems/RenderingSystem.h"
#include "../util/IntersectionUtil.h"
#include "PlacementHelper.h"

TouchInputManager* TouchInputManager::instance = 0;


TouchInputManager* TouchInputManager::Instance() {
	if (instance == 0) instance = new TouchInputManager();
	return instance;
}

void TouchInputManager::init(glm::vec2 pWorldSize, glm::vec2 pWindowSize) {
	worldSize = pWorldSize;
	windowSize = pWindowSize;
}

static glm::vec2 absoluteToCameraPos(const glm::vec2& pos) {
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
	glm::vec2 windowPos;

    for (int i=0; i<2; i++) {
    	wasTouching[i] = touching[i];
    	touching[i] = ptr->isTouching(i, &windowPos);
    	if (touching[i]) {
    		lastTouchedScreenPosition[i] = windowToWorld(windowPos, worldSize, windowSize);
            windowPos.x = windowPos.x / (float)PlacementHelper::WindowWidth - 0.5;
            windowPos.y = 0.5 - windowPos.y / (float)PlacementHelper::WindowHeight;
            lastTouchedPosition[i] = absoluteToCameraPos(windowPos);
    	}
    }
}

glm::vec2 TouchInputManager::windowToWorld(const glm::vec2& windowCoords, const glm::vec2& worldSize, const glm::vec2& windowSize) const {
	float x = (windowCoords.x / windowSize.x) * worldSize.x - worldSize.x * 0.5f;
	float y = ((windowSize.y - windowCoords.y) / windowSize.y) * worldSize.y - worldSize.y * 0.5f;
	return glm::vec2(x, y);
}
