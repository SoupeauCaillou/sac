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
    componentSerializer.add(new Property<bool>("enabled", OFFSET(enabled, bc)));
    componentSerializer.add(new Property<float>("over_size", OFFSET(overSize, bc), 0.001));
    componentSerializer.add(new Property<float>("vibration", OFFSET(vibration, bc), 0.001));
    componentSerializer.add(new Property<TextureRef>("texture_active", PropertyType::Texture, OFFSET(textureActive, bc), 0));
    componentSerializer.add(new Property<TextureRef>("texture_inactive", PropertyType::Texture, OFFSET(textureInactive, bc), 0));
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
    	auto* rc = theRenderingSystem.Get(e, false);
    	if (rc) {
    		UpdateButton(e, bt, touch, camerasAdaptedPos[(int) (rc->cameraBitMask / 2)]);
    	} else {
    		UpdateButton(e, bt, touch, theTouchInputManager.getTouchLastPosition(0));
    	}
    });
}

void ButtonSystem::UpdateButton(Entity entity, ButtonComponent* comp, bool touching, const glm::vec2& touchPos) {
	if (!comp->enabled) {
		comp->mouseOver = comp->clicked = comp->touchStartOutside = false;
		return;
	}

    comp->clicked = false;

    if (!touching)
        comp->touchStartOutside = false;

	const glm::vec2& pos = TRANSFORM(entity)->position;
	const glm::vec2& size = TRANSFORM(entity)->size;

	bool over = touching && IntersectionUtil::pointRectangle(touchPos, pos, size * comp->overSize, TRANSFORM(entity)->rotation);

	auto* rc = theRenderingSystem.Get(entity, false);
	if (rc) {
		if (comp->textureActive != InvalidTextureRef) {
			// Adapt texture to button state
			if (touching && over && !comp->touchStartOutside)
				rc->texture = comp->textureActive;
			else
				rc->texture = comp->textureInactive;
		}


	}
	if (comp->enabled && !comp->touchStartOutside) {
        if (comp->mouseOver) {
			if (touching) {
				comp->mouseOver = over;
			} else {
				if (!comp->touchStartOutside) {
					float t =TimeUtil::GetTime();
					// at least 200 ms between 2 clicks
					if (t - comp->lastClick > .2) {
						comp->lastClick = t;
						comp->clicked = true;

                         LOGI("Entity '" << theEntityManager.entityName(entity) << "' clicked");

                        if (vibrateAPI && comp->vibration > 0) {
                            vibrateAPI->vibrate(comp->vibration);
                        }
					}
				}
				comp->mouseOver = false;
			}
		} else {
			comp->touchStartOutside = touching & !over;
			comp->mouseOver = touching & over;
		}
	} else {
        comp->mouseOver = false;
    }
}
