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
		virtual bool isTouching(Vector2* windowCoords) const = 0;
};

#define theTouchInputManager (*TouchInputManager::Instance())

class TouchInputManager {
	private:
		static TouchInputManager* instance;
	public:
		static TouchInputManager* Instance();

		void init(Vector2 worldSize, Vector2 windowSize);

		bool wasTouched() const { return wasTouching; }

		bool isTouched() const { return touching; }
		const Vector2& getTouchLastPosition() const { return lastTouchedPosition; }

		void Update(float dt);

		void setNativeTouchStatePtr(NativeTouchState* p) { ptr = p; }

		Vector2 windowToWorld(const Vector2& windowCoords, const Vector2& worldSize, const Vector2& windowSize) const;
	private:
		NativeTouchState* ptr;

		bool wasTouching, touching;
		Vector2 lastTouchedPosition;

		Vector2 worldSize, windowSize;
};
