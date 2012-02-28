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
