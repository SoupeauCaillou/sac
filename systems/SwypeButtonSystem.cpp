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
#include "glm/gtx/projection.hpp"

INSTANCE_IMPL(SwypeButtonSystem);

SwypeButtonSystem::SwypeButtonSystem() : ComponentSystemImpl<SwypeButtonComponent>("SwypeButton") {
    /* nothing saved */
    vibrateAPI = 0;

    SwypeButtonComponent sc;
    componentSerializer.add(new Property<bool>("enabled", OFFSET(enabled, sc)));
    componentSerializer.add(new Property<float>("vibration", OFFSET(vibration, sc), 0.001));
    componentSerializer.add(new Property<bool>("animated", OFFSET(animated, sc)));
    componentSerializer.add(new Property<glm::vec2>("final_pos", OFFSET(finalPos, sc), glm::vec2(0.001, 0)));
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

    // if all active buttons have been idle for a while, give a hint to the player
    bool allIdle = true;
    std::vector<Entity> choices;
    theSwypeButtonSystem.forEachECDo([&] (Entity e, SwypeButtonComponent *bt) -> void {
        if (bt->enabled) {
            if (bt->activeIdleTime < 4) {
                allIdle = false;
            }
            if (allIdle) {
                choices.push_back(e);
            }
        }
    });

    if (allIdle && !choices.empty()) {
        // push one of them
        Entity pushed = choices[Random::Int(0, choices.size() - 1)];
        LOGI("idle, pushing " << theEntityManager.entityName(pushed));
        auto* comp = SWYPEBUTTON(pushed);

        glm::vec2 direction = glm::normalize(comp->finalPos - comp->idlePos);
        comp->animationPlaying = true;
        comp->speed = direction * glm::vec2(75.f);

        theSwypeButtonSystem.forEachECDo([&] (Entity, SwypeButtonComponent *bt) -> void {
            bt->activeIdleTime = 0;
        });
    }
}

void SwypeButtonSystem::UpdateSwypeButton(float dt, Entity entity, SwypeButtonComponent* comp, bool touching, const glm::vec2& touchPos) {
    if (!comp->enabled) {
        comp->mouseOver = 
            comp->touchStartOutside = 
            comp->clicked = false;
        return;
    }

    LOGF_IF(comp->idlePos == comp->finalPos, theEntityManager.entityName(entity) << " have the point as origin and final destination!");

    comp->clicked = false;

    const glm::vec2& pos = TRANSFORM(entity)->position;
    const glm::vec2& size = TRANSFORM(entity)->size;    

    bool over = touching && IntersectionUtil::pointRectangle(touchPos, pos, size, TRANSFORM(entity)->rotation);
    if (touching && (!over || (over && comp->touchStartOutside))) {
        comp->touchStartOutside = true;
    } else {
        comp->touchStartOutside = false;
    }

    glm::vec2 direction = glm::normalize(comp->finalPos - comp->idlePos);

    // Animation of button (to show it)
    if (comp->animated && glm::length(comp->idlePos - pos) < 0.01) {
        comp->activeIdleTime += dt;
    } else if ((!over && !comp->mouseOver) && comp->animationPlaying) {
        comp->speed += SteeringBehavior::arrive(
            pos, comp->speed, comp->idlePos, 10, 0.1);
    } else {
        comp->animationPlaying = false;
        comp->activeIdleTime = 0;
    }

    // User touches this button
    if (over) {
        comp->animationPlaying = false;
        if (!comp->mouseOver && !comp->touchStartOutside) {
            comp->mouseOver = true;
            comp->lastPos = touchPos;
        }
    }
    // if he's touching and was once on button
    if (touching && comp->mouseOver) {
        comp->speed = comp->speed * 0.5f + (glm::proj(touchPos - comp->lastPos, direction)/dt) * 0.5f;
        comp->lastPos = touchPos;
    } else {
        comp->mouseOver = false;
    }

    // update entity position with its speed
    TRANSFORM(entity)->position += comp->speed * dt;
    if (glm::min(comp->idlePos, comp->finalPos) == comp->idlePos) {
        TRANSFORM(entity)->position = glm::max(comp->idlePos, pos);
    } else {
        TRANSFORM(entity)->position = glm::min(comp->idlePos, pos);
    }

    //
    if (!touching && glm::length(comp->speed) < 1.f) {
        if (glm::length(pos) < glm::length(comp->finalPos)*0.5f) {
            comp->speed += SteeringBehavior::arrive(
                pos, comp->speed, comp->idlePos, 10, 0.1);
        } else {
            comp->speed += SteeringBehavior::arrive(
                pos, comp->speed, comp->finalPos, 10, 0);
        }
    }

    // check if the button is arrived at its final pos
    if (glm::length(pos - comp->idlePos) > glm::length(comp->finalPos - comp->idlePos) && !comp->animationPlaying) {
        LOGI("Button clicked !");
        comp->speed = glm::vec2(0.0f);
        comp->clicked = true;
        comp->mouseOver = false;
    }
}

            
