#pragma once

#include <glm/glm.hpp>


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

		bool wasTouched(int idx = 0) const { return wasTouching[idx]; }

		bool isTouched(int idx = 0) const { return touching[idx]; }

		bool hasMoved(int idx = 0) const { return moving[idx]; }

        bool hasClicked(int idx = 0) const { return clicked[idx]; }

        bool hasDoubleClicked(int idx = 0) const { return doubleclicked[idx]; }

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

		bool wasTouching[2], touching[2], moving[2], clicked[2], doubleclicked[2];
		glm::vec2 lastTouchedPosition[2], lastTouchedPositionScreen[2];
		float lastClickTime[2];

		glm::vec2 worldSize, windowSize;
};
