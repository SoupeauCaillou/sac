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
#pragma once

#include "base/TouchInputManager.h"
#include "base/MathUtil.h"
#include "base/Log.h"

#include "System.h"
#include "RenderingSystem.h"
#include "TransformationSystem.h"

class VibrateAPI;

struct ButtonComponent {
	ButtonComponent() : enabled(false), mouseOver(false), clicked(false), overSize(1) , vibration(0.035), lastClick(0) { }
	
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

