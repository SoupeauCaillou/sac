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

void TouchInputManager::Update(float dt __attribute__((unused)) ) {
	Vector2 windowPos;

	wasTouching = touching;
	touching = ptr->isTouching(&windowPos);
	if (touching) {
		lastTouchedPosition = windowToWorld(windowPos, worldSize, windowSize);
	}
}

Vector2 TouchInputManager::windowToWorld(const Vector2& windowCoords, const Vector2& worldSize, const Vector2& windowSize) const {
	float x = (windowCoords.X / windowSize.X) * worldSize.X - worldSize.X * 0.5f;
	float y = ((windowSize.Y - windowCoords.Y) / windowSize.Y) * worldSize.Y - worldSize.Y * 0.5f;
	return Vector2(x, y);
}
