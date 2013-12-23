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



#pragma once

#include "System.h"
#include "RenderingSystem.h"

struct ScrollingComponent {
	ScrollingComponent() :
        direction(0),
        speed(0),
        displaySize(0),
        show(false),
        opaqueType(RenderingComponent::FULL_OPAQUE) {}
    std::vector<std::string> images;
    glm::vec2 direction;
    float speed;
    glm::vec2 displaySize;
    // transitive rendering properties
    bool show;
    RenderingComponent::Opacity opaqueType;
};

#define theScrollingSystem ScrollingSystem::GetInstance()
#if SAC_DEBUG
#define SCROLLING(actor) theScrollingSystem.Get(actor,true,__FILE__,__LINE__)
#else
#define SCROLLING(actor) theScrollingSystem.Get(actor)
#endif
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
