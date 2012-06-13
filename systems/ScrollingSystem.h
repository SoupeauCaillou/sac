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

#include "base/MathUtil.h"
#include "base/Log.h"

#include "System.h"
#include <vector>

struct ScrollingComponent {
    std::vector<std::string> images;
    Vector2 direction;
    float speed;
    Vector2 displaySize;
    bool hide;
};

#define theScrollingSystem ScrollingSystem::GetInstance()
#define SCROLLING(actor) theScrollingSystem.Get(actor)
UPDATABLE_SYSTEM(Scrolling)

private:
struct ScrollingElement {
	Entity e[2];
	int imageIndex[2];
	bool hasBeenVisible[2];
};
void initScrolling(Entity e, ScrollingComponent* sc);
std::map<Entity, ScrollingElement> elements;
typedef std::map<Entity, ScrollingElement>::iterator EltIt;

};

