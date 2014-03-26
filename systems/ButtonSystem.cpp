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



#include "ButtonSystem.h"

#include "TransformationSystem.h"
#include "RenderingSystem.h"
#include "CameraSystem.h"

#include "base/TimeUtil.h"

#include "api/VibrateAPI.h"
#include "base/TouchInputManager.h"

#include "util/IntersectionUtil.h"

INSTANCE_IMPL(ButtonSystem);

ButtonSystem::ButtonSystem() : ComponentSystemImpl<ButtonComponent>("Button") {
    /* nothing saved */
    vibrateAPI = 0;

    ButtonComponent bc;
    componentSerializer.add(new Property<bool>(Murmur::Hash("enabled"), OFFSET(enabled, bc)));
    componentSerializer.add(new Property<float>(Murmur::Hash("over_size"), OFFSET(overSize, bc), 0.001f));
    componentSerializer.add(new Property<float>(Murmur::Hash("vibration"), OFFSET(vibration, bc), 0.001f));
    componentSerializer.add(new Property<float>(Murmur::Hash("trigger"), OFFSET(trigger, bc), 0.001f));
    componentSerializer.add(new Property<float>(Murmur::Hash("first_touch"), OFFSET(firstTouch, bc), 0.001f));
    componentSerializer.add(new Property<int>(Murmur::Hash("type"), OFFSET(type, bc)));
    componentSerializer.add(new Property<TextureRef>(Murmur::Hash("texture_active"), PropertyType::Texture, OFFSET(textureActive, bc), 0));
    componentSerializer.add(new Property<TextureRef>(Murmur::Hash("texture_inactive"), PropertyType::Texture, OFFSET(textureInactive, bc), 0));
}

void ButtonSystem::DoUpdate(float) {
    bool touch = theTouchInputManager.isTouched(0);

    std::vector<glm::vec2> camerasAdaptedPos;
    if (touch) {
        glm::vec2 pos = theTouchInputManager.getTouchLastPositionScreen(0);

        theCameraSystem.forEachECDo([pos, &camerasAdaptedPos] (Entity c, CameraComponent* cc) -> void {
            camerasAdaptedPos.resize(glm::max((int)camerasAdaptedPos.size(), cc->id + 1));
            camerasAdaptedPos[cc->id] = CameraSystem::ScreenToWorld(TRANSFORM(c), pos);
        });
    }


    theButtonSystem.forEachECDo([&] (Entity e, ButtonComponent *bt) -> void {
        const auto* rc = theRenderingSystem.Get(e, false);
        if (camerasAdaptedPos.empty()) {
            LOGT_EVERY_N(60000, "Warning... Gautier no-idea-what-doing fix!");
        }

        if (rc && ! camerasAdaptedPos.empty()) {
            UpdateButton(e, bt, touch, camerasAdaptedPos[(int) (rc->cameraBitMask / 2)]);
        } else {
            UpdateButton(e, bt, touch, theTouchInputManager.getTouchLastPosition(0));
        }
    });
}

void ButtonSystem::UpdateButton(Entity entity, ButtonComponent* comp, bool touching, const glm::vec2& touchPos) {
    if (!comp->enabled) {
        auto* rc = theRenderingSystem.Get(entity, false);
        if (rc && rc->show && rc->texture == InvalidTextureRef)
            rc->texture = comp->textureInactive;
        comp->mouseOver = comp->clicked = comp->touchStartOutside = false;
        return;
    }

    comp->clicked = false;

    if (!touching)
        comp->touchStartOutside = false;

    const auto* tc = TRANSFORM(entity);
    const glm::vec2& pos = tc->position;
    const glm::vec2& size = tc->size;

    bool over = touching && IntersectionUtil::pointRectangle(touchPos, pos, size * comp->overSize, tc->rotation);

    auto* rc = theRenderingSystem.Get(entity, false);
    if (rc) {
        if (comp->textureActive != InvalidTextureRef) {
            #ifdef SAC_DEBUG
            if (oldTexture.find(entity) != oldTexture.end() && rc->texture != oldTexture[entity])
                LOGW("Texture is changed in another place! Current=" << rc->texture << "!= old=" << oldTexture[entity]);
            #endif

            // Adapt texture to button state
            if (touching && over && !comp->touchStartOutside)
                rc->texture = comp->textureActive;
            else
                rc->texture = comp->textureInactive;

            #ifdef SAC_DEBUG
            oldTexture[entity] = rc->texture;
            #endif
        }
    }
    // If button is enabled and we have clicked on button at beginning
    if (comp->enabled && !comp->touchStartOutside) {
        // If we are clicking on the button
        if (comp->mouseOver) {
            if (touching) {
                comp->mouseOver = over;
                if (comp->type == ButtonComponent::LONGPUSH) {
                    float t =TimeUtil::GetTime();
                    if (comp->firstTouch == 0) {
                        comp->firstTouch = t;
                    }
                    LOGI_EVERY_N(100, t-comp->firstTouch);
                    if (t - comp->firstTouch > comp->trigger) {
                        comp->firstTouch = 0;
                        comp->lastClick = t;
                        comp->clicked = true;
                    }
                }
            } else {
                if (!comp->touchStartOutside) {
                    float t =TimeUtil::GetTime();
                    // at least 200 ms between 2 clicks

                    if (t - comp->lastClick > .2) {
                        if (comp->type == ButtonComponent::NORMAL) {
                            comp->lastClick = t;
                            comp->clicked = true;
                        }

                        LOGI("Entity '" << theEntityManager.entityName(entity) << "' clicked");

                        if (vibrateAPI && comp->vibration > 0) {
                            vibrateAPI->vibrate(comp->vibration);
                        }
                    }

                    comp->firstTouch = 0;
                }

                comp->mouseOver = false;
            }
        } else {
            comp->touchStartOutside = touching & !over;
            comp->mouseOver = touching & over;

            comp->firstTouch = 0;
        }
    } else {
        comp->mouseOver = false;
    }
}
