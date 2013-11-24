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
#pragma once

#include "System.h"

class VibrateAPI;

struct SwypeButtonComponent {
	SwypeButtonComponent() : mouseOver(false), enabled(false),
	lastPos(0.0f), idlePos(0.0f), animationPlaying(false), vibration(0.035f) { }

    ////// READ ONLY variables
    // States of button
    bool mouseOver;
    bool clicked, touchStartOutside;
    // Last position when entity was clicked
    glm::vec2 lastPos;
    // idle position
    glm::vec2 idlePos;
    //
    bool animationPlaying;
    //
    float activeIdleTime;
	////// END OF READ ONLY variables

	////// READ/WRITE variables
    // if true, entity is clickable
    bool enabled;
    // Speed of button
    glm::vec2 speed;
    // Swype direction
    glm::vec2 direction;
	////// END OF READ/WRITE variables

	////// WRITE ONLY variables
	// Vibration on clicked
	float vibration;
	// animated button
	bool animated;
	////// END OF WRITE ONLY variables
};

#define theSwypeButtonSystem SwypeButtonSystem::GetInstance()
#define SWYPEBUTTON(actor) theSwypeButtonSystem.Get(actor)
UPDATABLE_SYSTEM(SwypeButton)

public:
    VibrateAPI* vibrateAPI;
private:
	void UpdateSwypeButton(float dt, Entity entity, SwypeButtonComponent* comp, bool touching, const glm::vec2& touchPos);

};
