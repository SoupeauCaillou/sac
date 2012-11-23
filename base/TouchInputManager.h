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

#include "base/Vector2.h"

class NativeTouchState {
	public:
        virtual int maxTouchingCount() = 0;
		virtual bool isTouching(int index, Vector2* windowCoords) const = 0;
};

#define theTouchInputManager (*TouchInputManager::Instance())

class TouchInputManager {
	private:
		static TouchInputManager* instance;
	public:
		static TouchInputManager* Instance();

		void init(Vector2 worldSize, Vector2 windowSize);

		bool wasTouched(int idx) const { return wasTouching[idx]; }

		bool isTouched(int idx) const { return touching[idx]; }
		const Vector2& getTouchLastScreenPosition(int idx) const { return lastTouchedScreenPosition[idx]; }
        const Vector2& getTouchLastPosition(int idx) const { return lastTouchedPosition[idx]; }

		void Update(float dt);

		void setNativeTouchStatePtr(NativeTouchState* p) { ptr = p; }

		Vector2 windowToWorld(const Vector2& windowCoords, const Vector2& worldSize, const Vector2& windowSize) const;
	private:
		NativeTouchState* ptr;

		bool wasTouching[2], touching[2];
		Vector2 lastTouchedPosition[2], lastTouchedScreenPosition[2];

		Vector2 worldSize, windowSize;
};
