#pragma once

#include "System.h"

#include <list>

struct ZSQDComponent {
    ZSQDComponent() {}

    //new directions are added in this list
    std::list<glm::vec2> directions;

    glm::vec2 currentDirection;
    glm::vec2 linearVelocity;
};

#define theZSQDSystem ZSQDSystem::GetInstance()
#define ZSQD(actor) theZSQDSystem.Get(actor)
UPDATABLE_SYSTEM(ZSQD)
};
