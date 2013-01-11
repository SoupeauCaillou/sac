#pragma once

#include "System.h"
#include "RenderingSystem.h"

struct ScrollingComponent {
	ScrollingComponent() : opaqueType(RenderingComponent::NON_OPAQUE) {}
    std::vector<std::string> images;
    Vector2 direction;
    float speed;
    Vector2 displaySize;
    // transitive rendering properties
    bool hide;
    RenderingComponent::Opacity opaqueType;
    float opaqueSeparation;
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

