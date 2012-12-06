#include "ButtonSystem.h"
#include "base/TimeUtil.h"
#include "util/IntersectionUtil.h"
#include "../api/VibrateAPI.h"

INSTANCE_IMPL(ButtonSystem);

ButtonSystem::ButtonSystem() : ComponentSystemImpl<ButtonComponent>("Button") {
    /* nothing saved */
    vibrateAPI = 0;
}

void ButtonSystem::DoUpdate(float dt __attribute__((unused))) {
    bool touch = theTouchInputManager.isTouched(0);
	const Vector2& pos = theTouchInputManager.getTouchLastPosition(0);
    FOR_EACH_ENTITY_COMPONENT(Button, entity, bt)
		UpdateButton(entity, bt, touch, pos);
	}
}

void ButtonSystem::UpdateButton(Entity entity, ButtonComponent* comp, bool touching, const Vector2& touchPos) {
	if (!comp->enabled) {
		comp->mouseOver = comp->clicked = comp->touchStartOutside = false;
		return;
	}

    comp->clicked = false;

    if (!touching && !comp->mouseOver)
        return;

	const Vector2& pos = TRANSFORM(entity)->worldPosition;
	const Vector2& size = TRANSFORM(entity)->size;

	bool over = IntersectionUtil::pointRectangle(touchPos, pos, size * comp->overSize);
	if (comp->enabled) {
        if (comp->mouseOver) {
			if (touching) {
				comp->mouseOver = over;
			} else {
				if (!comp->touchStartOutside) {
					float t =TimeUtil::getTime();
					// at least 200 ms between 2 clicks
					if (t - comp->lastClick > .2) {
						comp->lastClick = t;
						comp->clicked = true;
                     std::cout << entity << " -> clicked" <<std::endl;

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
	}
}
