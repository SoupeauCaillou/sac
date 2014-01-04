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

#include <glm/glm.hpp>
#include <vector>

#define theJoystickManager (*JoystickManager::Instance())

namespace JoystickButton {
	enum Enum {
		GREEN = 0,
		RED,
		BLUE,
		YELLOW,
		LB,
		RB,
		BACK,
		START,
		XBOX,
		LEFTCLICK,
		RIGHTCLICK,
		TOTAL
	};
}

namespace JoystickPad {
	enum Enum {
		LEFT = 0,
		RIGHT,
		TOTAL,
	};
}

struct JoystickState {
	bool clicked[JoystickButton::TOTAL];
	bool doubleclicked[JoystickButton::TOTAL];
	float lastClickTime[JoystickButton::TOTAL];

	glm::vec2 lastDirection[JoystickPad::TOTAL];

	JoystickState() {
		for (unsigned b = 0; b < JoystickButton::TOTAL; ++b) {
			clicked[b] = doubleclicked[b] = false;
			lastClickTime[b] = 0;
		}
	}

	void* joystickPtr;
};

class JoystickManager {
	private:
		static JoystickManager* instance;
	public:
		static JoystickManager* Instance();
		static void DestroyInstance();
		
        bool hasClicked(int idx, JoystickButton::Enum btn) const { return joysticks[idx].clicked[btn]; }

        bool hasDoubleClicked(int idx, JoystickButton::Enum btn) const { return joysticks[idx].doubleclicked[btn]; }

        void resetDoubleClick(int idx, JoystickButton::Enum btn);

        const glm::vec2& getPadDirection(int idx, JoystickPad::Enum pad) const { return joysticks[idx].lastDirection[pad]; }

		void Update(float dt);

		int eventSDL(void* event);
	private:
		std::vector<JoystickState> joysticks;
};
