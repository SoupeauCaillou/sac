#pragma once

#include "base/Vector2.h"

typedef bool (*NativeTouchStatePtr) (Vector2* windowCoords);

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

		void setNativeTouchStatePtr(NativeTouchStatePtr p) { ptr = p; }
			
		Vector2 windowToWorld(const Vector2& windowCoords, const Vector2& worldSize, const Vector2& windowSize) const;
	private:
		NativeTouchStatePtr ptr;

		bool wasTouching, touching;
		Vector2 lastTouchedPosition;

		Vector2 worldSize, windowSize;	
};

