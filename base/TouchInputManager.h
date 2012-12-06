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
