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
#include "SwypeButtonSystem.h"

#include "TransformationSystem.h"
#include "RenderingSystem.h"
#include "CameraSystem.h"

#include "steering/SteeringBehavior.h"

#include "base/TimeUtil.h"

#include "api/VibrateAPI.h"
#include "base/TouchInputManager.h"

#include "util/IntersectionUtil.h"
#include "util/Random.h"

#include "glm/glm.hpp"

INSTANCE_IMPL(SwypeButtonSystem);

SwypeButtonSystem::SwypeButtonSystem() : ComponentSystemImpl<SwypeButtonComponent>("SwypeButton") {
    /* nothing saved */
    vibrateAPI = 0;

    SwypeButtonComponent sc;
    componentSerializer.add(new Property<bool>("enabled", OFFSET(enabled, sc)));
    componentSerializer.add(new Property<float>("vibration", OFFSET(vibration, sc), 0.001));
    componentSerializer.add(new Property<bool>("animated", OFFSET(animated, sc)));
    componentSerializer.add(new Property<glm::vec2>("direction", OFFSET(direction, sc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<glm::vec2>("idle_pos", OFFSET(idlePos, sc), glm::vec2(0.001, 0)));
    
}

void SwypeButtonSystem::DoUpdate(float dt) {
    bool touch = theTouchInputManager.isTouched(0);

    std::vector<glm::vec2> camerasAdaptedPos;
    if (touch) {
        glm::vec2 pos = theTouchInputManager.getTouchLastPositionScreen(0);

        theCameraSystem.forEachECDo([pos, &camerasAdaptedPos] (Entity c, CameraComponent* cc) -> void {
            camerasAdaptedPos.resize(glm::max((int)camerasAdaptedPos.size(), cc->id + 1));
            camerasAdaptedPos[cc->id] = CameraSystem::ScreenToWorld(TRANSFORM(c), pos);
        });
    }


    theSwypeButtonSystem.forEachECDo([&] (Entity e, SwypeButtonComponent *bt) -> void {
        const auto* rc = theRenderingSystem.Get(e, false);
        if (rc) {
            UpdateSwypeButton(dt, e, bt, touch, camerasAdaptedPos[(int) (rc->cameraBitMask >> 1)]);
        } else {
            UpdateSwypeButton(dt, e, bt, touch, theTouchInputManager.getTouchLastPosition(0));
        }
    });

}

void SwypeButtonSystem::UpdateSwypeButton(float dt, Entity entity, SwypeButtonComponent* comp, bool touching, const glm::vec2& touchPos) {
    if (!comp->enabled) {
        comp->mouseOver = comp->clicked = false;
        return;
    }

    comp->clicked = false;

    const glm::vec2& pos = TRANSFORM(entity)->position;
    const glm::vec2& size = TRANSFORM(entity)->size;    

    bool over = touching && IntersectionUtil::pointRectangle(touchPos, pos, size, TRANSFORM(entity)->rotation);

    // Animation of button (to show it)
    if (comp->animated && glm::length(comp->idlePos - pos) < 0.01) {
        comp->activeIdleTime += dt;
        if (comp->activeIdleTime > 4) {
            LOGI("idle, pushing " << theEntityManager.entityName(entity));
            comp->animationPlaying = true;
            comp->speed = comp->direction * glm::vec2(10.f);
            comp->activeIdleTime = 0;
        }
    } else if ((!over && !comp->mouseOver) && comp->animationPlaying) {
        LOGI("deccelerate : " << comp->speed << " + " << -comp->direction * comp->speed * glm::vec2(10.f * dt));
        comp->speed = comp->speed - comp->direction * comp->speed * glm::vec2(10.f * dt);
    } else {
        comp->animationPlaying = false;
        comp->activeIdleTime = 0;
    }

    // User touches this button
    if (over) {
        if (!comp->mouseOver) {
            comp->mouseOver = true;
            comp->lastPos = touchPos;
        }
    }
    // if he's touching and was once on button
    if (touching && comp->mouseOver) {
        comp->speed = comp->direction * glm::distance(touchPos, comp->lastPos) / dt;
        comp->lastPos = touchPos;
    } else {
        comp->mouseOver = false;
    }

    TRANSFORM(entity)->position += comp->speed * dt;

    glm::vec2 d = glm::normalize(TRANSFORM(entity)->position - comp->idlePos);
    LOGI("current direction : "<< d << " set direction " << comp->direction);
    if (d == glm::normalize(-comp->direction))
        TRANSFORM(entity)->position = comp->idlePos;

    glm::vec2 cameraSize;
    glm::vec2 cameraPosition;
    const int entityMask = RENDERING(entity)->cameraBitMask;
    theCameraSystem.forEachECDo([&entityMask, &cameraSize, &cameraPosition] (Entity c, CameraComponent* cc) -> void {
        if (entityMask == cc->id) {
            cameraPosition = TRANSFORM(c)->position;
            cameraSize = TRANSFORM(c)->size;
        }
    });

    if (!theRenderingSystem.isVisible(entity)) {
        LOGI("Button clicked !");
        comp->speed = glm::vec2(0.0f);
        comp->clicked = true;
        TRANSFORM(entity)->position = comp->idlePos;
    }

    if (!touching && glm::length(comp->speed) < 1.f) {
        comp->speed = glm::vec2(0.0f);
        TRANSFORM(entity)->position = comp->idlePos;
    }

}

            
