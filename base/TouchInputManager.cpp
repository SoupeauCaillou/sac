#include "TouchInputManager.h"
#include "Log.h"

TouchInputManager* TouchInputManager::instance = 0;

				
TouchInputManager* TouchInputManager::Instance() {
	if (instance == 0) instance = new TouchInputManager();
	return instance;
}

void TouchInputManager::init(Vector2 pWorldSize, Vector2 pWindowSize) {
	worldSize = pWorldSize;
	windowSize = pWindowSize;
}

void TouchInputManager::Update(float dt) {
	Vector2 windowPos;

	wasTouching = touching;
	touching = ptr(&windowPos);
	if (touching) {
		lastTouchedPosition = windowToWorld(windowPos, worldSize, windowSize);
	}
}

Vector2 TouchInputManager::windowToWorld(const Vector2& windowCoords, const Vector2& worldSize, const Vector2& windowSize) const {
	float x = (windowCoords.X / windowSize.X) * worldSize.X - worldSize.X * 0.5f;
	float y = ((windowSize.Y - windowCoords.Y) / windowSize.Y) * worldSize.Y - worldSize.Y * 0.5f;
	return Vector2(x, y);
}

