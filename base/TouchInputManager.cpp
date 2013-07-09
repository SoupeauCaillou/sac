#include <glm/gtx/rotate_vector.hpp>
#include "TouchInputManager.h"
#include "Log.h"
#include "../systems/RenderingSystem.h"
#include "../systems/TransformationSystem.h"
#include "../systems/CameraSystem.h"
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

void TouchInputManager::Update(float) {
    Entity camera = 0;
    const std::vector<Entity> cameras = theCameraSystem.RetrieveAllEntityWithComponent();
    for (auto c: cameras) {
        if (CAMERA(c)->fb == DefaultFrameBufferRef) {
            camera = c;
            break;
        }
    }
    if (!camera) {
        LOGW("No camera defined -> no input handling");
        return;
    }
    TransformationComponent* tc = TRANSFORM(camera);

    const glm::vec2 windowSize(theRenderingSystem.screenW, theRenderingSystem.screenH);
    glm::vec2 coords;
    const unsigned pointers = ptr->maxTouchingCount();

    for (unsigned i=0; i<pointers; i++) {
    	wasTouching[i] = touching[i];
        moving[i] = ptr->isMoving(i);
        touching[i] = ptr->isTouching(i, &coords);
        if (touching[i]) {
            // convert window coordinates -> world coords
            lastTouchedPosition[i] = windowToWorld(coords, tc);
    	}
    }
}

glm::vec2 TouchInputManager::windowToWorld(const glm::vec2& windowCoords, const TransformationComponent* cameraTrans) const {
    glm::vec2 camLocal;
	camLocal.x = (windowCoords.x / theRenderingSystem.windowW) * cameraTrans->size.x
        - cameraTrans->size.x * 0.5f;
	camLocal.y = ((theRenderingSystem.windowH - windowCoords.y) / theRenderingSystem.windowH) * cameraTrans->size.y
        - cameraTrans->size.y * 0.5f;
	return cameraTrans->position + glm::rotate(camLocal, cameraTrans->rotation);
}
