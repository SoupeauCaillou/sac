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
#include "base/Log.h"
#include "PlacementHelper.h"
#include "util/ReplayManager.h"

#if SAC_DEBUG
#include "../systems/AnchorSystem.h"
#endif
#if SAC_INGAME_EDITORS
#include "util/LevelEditor.h"
#endif

#include <glm/gtx/norm.hpp>

TouchInputManager* TouchInputManager::instance = 0;


TouchInputManager* TouchInputManager::Instance() {
    if (instance == 0) instance = new TouchInputManager();
    return instance;
}

void TouchInputManager::init(glm::vec2 pWorldSize, glm::vec2 pWindowSize) {
    camera = 0;
    worldSize = pWorldSize;
    windowSize = pWindowSize;
#if SAC_DEBUG
    for (int i=0; i<MAX_TOUCH_POINT; i++) {
        debugState[i] = 0;
    }
#endif

    if (!theReplayManager.isReplayModeEnabled())
        theReplayManager.saveMaxTouchingCount(ptr->maxTouchingCount());
}

void TouchInputManager::setCamera(Entity c) {
    camera = c;
}

#if SAC_DEBUG
void TouchInputManager::activateDebug(Entity camera) {
    for (int i=0; i<MAX_TOUCH_POINT; i++) {
        debugState[i] = theEntityManager.CreateEntity(HASH("debug_input", 0x0));
        ADD_COMPONENT(debugState[i], Transformation);
        ADD_COMPONENT(debugState[i], Anchor);
        ANCHOR(debugState[i])->parent = camera;
        ANCHOR(debugState[i])->z = 0.9999f - TRANSFORM(camera)->z;
        ADD_COMPONENT(debugState[i], Rendering);
        RENDERING(debugState[i])->show = 1;
    }
}
#endif

void TouchInputManager::Update() {
    if (!camera) {
        LOGW_EVERY_N(60, "No camera defined -> no input handling");
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
            if (i == 0)
                LOGI_EVERY_N(30, __(lastTouchedPositionScreen[i]));
            lastTouchedPosition[i] = windowToWorld(coords, tc);

            if (!wasTouching[i]) {
                onTouchPosition[i] = lastTouchedPositionScreen[i];
            }
        }
        #if SAC_DESKTOP
        {
            lastOverPositionScreen[i] = windowToScreen(coords);
            lastOverPosition[i] = windowToWorld(coords, tc);
        }
        #endif

        // first click condition: was touched + is released
        if (!touching[i] && wasTouching[i]) {
            // click is valid if ~no move between on touch and on release event
            clicked[i] = (glm::distance2(lastTouchedPositionScreen[i], onTouchPosition[i]) < 0.005);

            if (clicked[i]) {
                float t = TimeUtil::GetTime();
                glm::vec2 pos = lastTouchedPositionScreen[i];

                if ((t - lastClickTime[i] < 0.3) && glm::distance2(pos, lastClickPosition[i]) < 0.005) {
                    doubleclicked[i] = true;
                    LOGV(1, "DOUBLE CLICKED("<< i << ") TOO!");
                } else {
                    lastClickPosition[i] = pos;
                    doubleclicked[i] = false;
                    LOGV(1, "CLICK(" << i << ") at " << t << ": " << pos);
                }
                lastClickTime[i] = t;
            } else {
                doubleclicked[i] = false;
            }
        } else {
            clicked[i] = doubleclicked[i] = false;
        }

        if (theReplayManager.isReplayModeEnabled()) {
            theReplayManager.saveIsTouching(i, touching[i], coords);
        }
    }


#if SAC_DEBUG
    if (!debugState[0])
        return;
    const auto* cam = TRANSFORM(ANCHOR(debugState[0])->parent);
    for (int i=0; i<MAX_TOUCH_POINT; i++) {
        TRANSFORM(debugState[i])->size = glm::vec2(0.05 * cam->size.y);

        ANCHOR(debugState[i])->position =
            AnchorSystem::adjustPositionWithCardinal(
                cam->size * glm::vec2(-0.5, 0.5) - glm::vec2(0, TRANSFORM(debugState[i])->size.y * i),
                TRANSFORM(debugState[i])->size,
                Cardinal::NW);

        if (touching[i])
            RENDERING(debugState[i])->color = Color(0, 1, 0);
        else
            RENDERING(debugState[i])->color = Color(1, 0, 0);
    }
#endif
}

void TouchInputManager::resetState() {
    for (int i=0; i<MAX_TOUCH_POINT; i++) {
        touching[i] = wasTouching[i] = clicked[i] = doubleclicked[i] = false;
        lastClickTime[i] = 0;
    }
}

void TouchInputManager::resetDoubleClick(int idx) {
    lastClickTime[idx] = 0;
}

glm::vec2 TouchInputManager::windowToScreen(const glm::vec2& windowCoords) const {
#if SAC_INGAME_EDITORS
    return glm::vec2(windowCoords.x / (LevelEditor::DebugAreaWidth + theRenderingSystem.windowW) - 0.5,
        (theRenderingSystem.windowH + LevelEditor::DebugAreaHeight - windowCoords.y) / (theRenderingSystem.windowH + LevelEditor::DebugAreaHeight) - 0.5);
#else
    return glm::vec2(windowCoords.x / theRenderingSystem.windowW - 0.5,
        (theRenderingSystem.windowH - windowCoords.y) / theRenderingSystem.windowH - 0.5);
#endif
}

glm::vec2 TouchInputManager::windowToWorld(const glm::vec2& windowCoords, const TransformationComponent* cameraTrans) const {
    glm::vec2 camLocal;
#if SAC_INGAME_EDITORS
    glm::vec2 c (windowCoords - LevelEditor::GameViewPosition());
    camLocal.x = (c.x / theRenderingSystem.windowW) * cameraTrans->size.x
        - cameraTrans->size.x * 0.5f;
    camLocal.y = ((theRenderingSystem.windowH - c.y) / theRenderingSystem.windowH) * cameraTrans->size.y
        - cameraTrans->size.y * 0.5f;
#else
    camLocal.x = (windowCoords.x / theRenderingSystem.windowW) * cameraTrans->size.x
        - cameraTrans->size.x * 0.5f;
    camLocal.y = ((theRenderingSystem.windowH - windowCoords.y) / theRenderingSystem.windowH) * cameraTrans->size.y
        - cameraTrans->size.y * 0.5f;
#endif
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
