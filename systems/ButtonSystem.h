#pragma once

#include "System.h"

class VibrateAPI;

struct ButtonComponent {
	ButtonComponent() : enabled(false), mouseOver(false), clicked(false), overSize(1.f) , vibration(0.035f), lastClick(0.f) { }

	bool enabled;
	bool mouseOver;
	bool clicked, touchStartOutside;
    float overSize, vibration;
    float lastClick;
};

#define theButtonSystem ButtonSystem::GetInstance()
#define BUTTON(actor) theButtonSystem.Get(actor)
UPDATABLE_SYSTEM(Button)

public:
    VibrateAPI* vibrateAPI;
private:
	void UpdateButton(Entity entity, ButtonComponent* comp, bool touching, const Vector2& touchPos);
};

