#include "AndroidNativeTouchState.h"
#include "sacjnilib.h"
#include "base/Log.h"

AndroidNativeTouchState::AndroidNativeTouchState(GameHolder* h) {
	holder = h;
}

int AndroidNativeTouchState::maxTouchingCount() {
	return holder->input.size();
}

bool AndroidNativeTouchState::isTouching (int index, glm::vec2* windowCoords) {
	// map stable order ?
	std::map<int, GameHolder::__input>::iterator it = holder->input.begin();
	for (int i=0; i<index && it!=holder->input.end(); ++it, i++)
		if (it == holder->input.end())
			return false;

	windowCoords->x = it->second.x;
	windowCoords->y = it->second.y;

	return it->second.touching;
}

bool AndroidNativeTouchState::isMoving (int index) {
	// map stable order ?
	std::map<int, GameHolder::__input>::iterator it = holder->input.begin();
	for (int i=0; i<index && it!=holder->input.end(); ++it, i++)
		if (it == holder->input.end())
			return false;
	GameHolder::__input& input = it->second;
	const bool result = input.moving;
	// clear status
	if (result) {
		input.moving = false;
	}
	return result;
}

