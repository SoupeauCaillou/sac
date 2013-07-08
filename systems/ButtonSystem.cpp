#include "ButtonSystem.h"

#include "TransformationSystem.h"
#include "RenderingSystem.h"

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
	const glm::vec2& pos = theTouchInputManager.getTouchLastPosition(0);

    theButtonSystem.forEachECDo([&] (Entity e, ButtonComponent *bt) -> void {
    	UpdateButton(e, bt, touch, pos);
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

	bool over = IntersectionUtil::pointRectangle(touchPos, pos, size * comp->overSize);
	if (theRenderingSystem.Get(entity, false)) {
		if (comp->textureActive != InvalidTextureRef) {
			if (touching && over)
				RENDERING(entity)->texture = comp->textureActive;
			else
				RENDERING(entity)->texture = comp->textureInactive;
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
