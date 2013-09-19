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
            lastTouchedPositionScreen[i] = windowToScreen(coords);
            lastTouchedPosition[i] = windowToWorld(coords, tc);
    	}

        clicked[i] = !touching[i] && wasTouching[i];
        if (clicked[i]) {
            float t = TimeUtil::GetTime();
            doubleclicked[i] = (t - lastClickTime[i] < 0.5);
            lastClickTime[i] = t;
            LOGI("CLICK(" << i << ") at " << t);
            if (doubleclicked[i])
                LOGI("DOUBLE CLICKED("<< i << ") TOO!");
        } else {
            doubleclicked[i] = false;
        }
    }
}

glm::vec2 TouchInputManager::windowToScreen(const glm::vec2& windowCoords) const {
    return glm::vec2(windowCoords.x / theRenderingSystem.windowW - 0.5,
        (theRenderingSystem.windowH - windowCoords.y) / theRenderingSystem.windowH - 0.5);
}

glm::vec2 TouchInputManager::windowToWorld(const glm::vec2& windowCoords, const TransformationComponent* cameraTrans) const {
    glm::vec2 camLocal;
	camLocal.x = (windowCoords.x / theRenderingSystem.windowW) * cameraTrans->size.x
        - cameraTrans->size.x * 0.5f;
	camLocal.y = ((theRenderingSystem.windowH - windowCoords.y) / theRenderingSystem.windowH) * cameraTrans->size.y
        - cameraTrans->size.y * 0.5f;
	return cameraTrans->position + glm::rotate(camLocal, cameraTrans->rotation);
}

#if !ANDROID
#include "app/MouseNativeTouchState.h"
int TouchInputManager::getWheel() const {
    int& r = static_cast<MouseNativeTouchState*>(ptr)->wheel;
    //hum
    int s = r;
    r = 0;
    return s;
}
#endif