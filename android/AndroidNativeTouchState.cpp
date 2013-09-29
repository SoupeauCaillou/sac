/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "AndroidNativeTouchState.h"
#include "sacjnilib.h"
#include "base/Log.h"

AndroidNativeTouchState::AndroidNativeTouchState(GameHolder* h) {
	holder = h;
}

int AndroidNativeTouchState::maxTouchingCount() {
	return 2; // holder->input.size();
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

