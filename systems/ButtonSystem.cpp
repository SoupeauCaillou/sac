/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "ButtonSystem.h"
#include "base/TimeUtil.h"
#include "util/IntersectionUtil.h"

INSTANCE_IMPL(ButtonSystem);

ButtonSystem::ButtonSystem() : ComponentSystemImpl<ButtonComponent>("Button") {
    /* nothing saved */
}

void ButtonSystem::DoUpdate(float dt) {
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

	const Vector2& pos = TRANSFORM(entity)->worldPosition;
	const Vector2& size = TRANSFORM(entity)->size;

	bool over = IntersectionUtil::pointRectangle(touchPos, pos, size * comp->overSize);
	comp->clicked = false;

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
					}
				}
				comp->mouseOver = false;
			}
		} else {
			if (touching && !over) {
				comp->touchStartOutside = true;
			}
			comp->mouseOver = touching & over;
		}
	}
	if (!touching)
		comp->touchStartOutside = false;
}
