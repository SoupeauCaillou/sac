#include "ButtonSystem.h"
#include "base/TouchInputManager.h"
#include "base/MathUtil.h"
#include "RenderingSystem.h"
#include "TransformationSystem.h"


	INSTANCE_IMPL(ButtonSystem);
	
	ButtonSystem::ButtonSystem() : ComponentSystemImpl<ButtonComponent>("button") { }
	
	void ButtonSystem::DoUpdate(float dt) {
		bool touch = theTouchInputManager.isTouched();
		const Vector2& pos = theTouchInputManager.getTouchLastPosition();
				
		for(std::map<Entity, ButtonComponent*>::iterator jt=components.begin(); jt!=components.end(); ++jt) {
			UpdateButton((*jt).first, (*jt).second, touch, pos);
		}
	}
	
	void ButtonSystem::UpdateButton(Entity entity, ButtonComponent* comp, bool touching, const Vector2& touchPos) {
		if (!comp->enabled) {
			comp->mouseOver = comp->clicked = comp->touchStartOutside = false;
			return;
		}

		const Vector2& pos = TRANSFORM(entity)->worldPosition;
		const Vector2& size = RENDERING(entity)->size;

		bool over = inside(touchPos, pos, size);
		
		if (comp->enabled) {
			if (comp->mouseOver) {
				if (touching) {
					comp->mouseOver = over;
				} else {
					if (!comp->touchStartOutside) {
						std::cout << entity << ": clicked" << std::endl;
						comp->clicked = true;
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

	bool ButtonSystem::inside(const Vector2& testPoint, const Vector2& rectPos, const Vector2& rectSize) {
 		return (MathUtil::Abs(rectPos.X - testPoint.X) < rectSize.X * 0.5 &&
			MathUtil::Abs(rectPos.Y - testPoint.Y) < rectSize.Y * 0.5);
	}
