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

#if SAC_DEBUG
#include "EntityManager.h"
#endif

#define MAX_TOUCH_POINT 2

class NativeTouchState {
	public:
        virtual int maxTouchingCount() = 0;
		virtual bool isTouching(int index, glm::vec2* windowCoords) = 0;
		virtual bool isMoving (int index) = 0;
};

struct TransformationComponent;

#define theTouchInputManager (*TouchInputManager::Instance())

class TouchInputManager {
	private:
		static TouchInputManager* instance;
	public:
		static TouchInputManager* Instance();

		void init(glm::vec2 worldSize, glm::vec2 windowSize);

#if SAC_DEBUG
		void activateDebug(Entity camera);
#endif
		bool wasTouched(int idx = 0) const { return wasTouching[idx]; }

		bool isTouched(int idx = 0) const { return touching[idx]; }

		bool hasMoved(int idx = 0) const { return moving[idx]; }

        bool hasClicked(int idx = 0) const { return clicked[idx]; }

        bool hasDoubleClicked(int idx = 0) const { return doubleclicked[idx]; }

        void resetDoubleClick(int idx = 0);

        const glm::vec2& getTouchLastPosition(int idx = 0) const { return lastTouchedPosition[idx]; }

        const glm::vec2& getTouchLastPositionScreen(int idx = 0) const { return lastTouchedPositionScreen[idx]; }

		void Update(float dt);

		void setNativeTouchStatePtr(NativeTouchState* p) { ptr = p; }

		glm::vec2 windowToWorld(const glm::vec2& windowCoords, const TransformationComponent* cameraTrans) const;

		glm::vec2 windowToScreen(const glm::vec2& windowCoords) const;

#if !ANDROID
		int getWheel() const;
#endif
	private:
		NativeTouchState* ptr;

		bool wasTouching[MAX_TOUCH_POINT], touching[MAX_TOUCH_POINT], moving[MAX_TOUCH_POINT], clicked[MAX_TOUCH_POINT], doubleclicked[MAX_TOUCH_POINT];
		glm::vec2 lastTouchedPosition[MAX_TOUCH_POINT], lastTouchedPositionScreen[MAX_TOUCH_POINT], lastClickPosition[MAX_TOUCH_POINT], onTouchPosition[MAX_TOUCH_POINT];
		float lastClickTime[MAX_TOUCH_POINT];

		glm::vec2 worldSize, windowSize;

#if SAC_DEBUG
		Entity debugState[MAX_TOUCH_POINT];
#endif
};
