#pragma once

#include <glm/glm.hpp>


class NativeTouchState {
	public:
        virtual int maxTouchingCount() = 0;
		virtual bool isTouching(int index, glm::vec2* windowCoords) const = 0;
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

        bool hasClicked(int idx = 0) const { return !touching[idx] && wasTouching[idx]; }

        const glm::vec2& getTouchLastPosition(int idx = 0) const { return lastTouchedPosition[idx]; }

		void Update(float dt);

		void setNativeTouchStatePtr(NativeTouchState* p) { ptr = p; }

		glm::vec2 windowToWorld(const glm::vec2& windowCoords, const TransformationComponent* cameraTrans) const;
	private:
		NativeTouchState* ptr;

		bool wasTouching[2], touching[2];
		glm::vec2 lastTouchedPosition[2];

		glm::vec2 worldSize, windowSize;
};
