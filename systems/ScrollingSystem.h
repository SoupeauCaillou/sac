#pragma once

#include "base/MathUtil.h"
#include "base/Log.h"

#include "System.h"
#include <vector>

struct ScrollingComponent {
    std::vector<std::string> images;
    Vector2 speed;
    Vector2 displaySize;
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

