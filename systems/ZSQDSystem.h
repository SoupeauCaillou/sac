#pragma once

#include "System.h"
#include "TransformationSystem.h"

#include <list>

struct ZSQDComponent {
    ZSQDComponent() : currentSpeed(0.f), maxSpeed(1.f), frictionCoeff(0.1f), newDirectionCoeff(0.01f), lateralMove(true), rotateToFaceDirection(false), rotationSpeed(5), rotationSpeedStopped(10) {}

    //new directions are added in this list
    std::list<glm::vec2> directions;

    //use this when you are using a vector, eg not dependent from the point position
    void addDirectionVector(glm::vec2 v) { directions.push_front(v); }
    //use this when the position vector is the target point
    void addDirectionPoint(Entity e, glm::vec2 pos) { directions.push_front(pos - TRANSFORM(e)->position); }

    //currentDirection of the entity, should not be changed
    glm::vec2 currentDirection;
    //same as above
    float currentSpeed;

    //max speed of the entity in range [0, +oo[
    float maxSpeed;

    //how much the speed will decrease per sec
    //in range [0, 1], relative to maxSpeed (0.1 is 10% of maxSpeed)
    float frictionCoeff;

    //how much the new direction will influences the direction (0: don't influence at all, 1: it's the new direction)
    //in range [0, 1]
    float newDirectionCoeff;

    bool lateralMove, rotateToFaceDirection;

    float rotationSpeed, rotationSpeedStopped;
};

#define theZSQDSystem ZSQDSystem::GetInstance()
#define ZSQD(actor) theZSQDSystem.Get(actor)
UPDATABLE_SYSTEM(ZSQD)
};
