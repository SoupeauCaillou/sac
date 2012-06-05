#pragma once

#include "base/TouchInputManager.h"
#include "base/MathUtil.h"
#include "base/Log.h"

#include "System.h"
#include "RenderingSystem.h"
#include "TransformationSystem.h"


struct ButtonComponent {
	ButtonComponent() : enabled(true), mouseOver(false), clicked(false), overSize(1) , lastClick(0) { }
	
	bool enabled;
	bool mouseOver;
	bool clicked, touchStartOutside;
    float overSize;
    float lastClick;
};

#define theButtonSystem ButtonSystem::GetInstance()
#define BUTTON(actor) theButtonSystem.Get(actor)
UPDATABLE_SYSTEM(Button)
		
private:
	void UpdateButton(Entity entity, ButtonComponent* comp, bool touching, const Vector2& touchPos);

public:
	static bool inside(const Vector2& testPoint, const Vector2& rectPos, const Vector2& rectSize);
};

