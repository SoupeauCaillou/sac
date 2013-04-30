#pragma once

#include "System.h"
#include "TransformationSystem.h"

#include <list>

struct ZSQDComponent {
    ZSQDComponent() : currentSpeed(0.f), maxSpeed(1.f), frictionCoeff(0.1f) {}

    //new directions are added in this list
    std::list<glm::vec2> directions;
    void addDirectionVector(glm::vec2 v) { directions.push_front(v); }
    void addDirectionPoint(Entity e, glm::vec2 pos) { directions.push_front(pos - TRANSFORM(e)->worldPosition); }

    glm::vec2 currentDirection;
    float currentSpeed;
    float maxSpeed;

    //in range [0, 1], relative to maxSpeed (0.1 is 10% of maxSpeed)
    float frictionCoeff;
};

#define theZSQDSystem ZSQDSystem::GetInstance()
#define ZSQD(actor) theZSQDSystem.Get(actor)
UPDATABLE_SYSTEM(ZSQD)
};
