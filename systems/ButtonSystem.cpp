#include "ButtonSystem.h"
#include "base/TimeUtil.h"
#include "util/IntersectionUtil.h"
#include "../api/VibrateAPI.h"
#include "base/TouchInputManager.h"

#include "TransformationSystem.h"
INSTANCE_IMPL(ButtonSystem);

ButtonSystem::ButtonSystem() : ComponentSystemImpl<ButtonComponent>("Button") {
    /* nothing saved */
    vibrateAPI = 0;

    ButtonComponent bc;
    componentSerializer.add(new Property<bool>("enabled", OFFSET(enabled, bc)));
}

void ButtonSystem::DoUpdate(float) {
    bool touch = theTouchInputManager.isTouched(0);
	const glm::vec2& pos = theTouchInputManager.getTouchLastPosition(0);
    FOR_EACH_ENTITY_COMPONENT(Button, entity, bt)
		UpdateButton(entity, bt, touch, pos);
	}
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

	bool over = IntersectionUtil::pointRectangle(touchPos, pos, size * comp->overSize);
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
