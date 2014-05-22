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

#if !DISABLE_ZSQD_SYSTEM
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
#if SAC_DEBUG
#define ZSQD(actor) theZSQDSystem.Get(actor,true,__FILE__,__LINE__)
#else
#define ZSQD(actor) theZSQDSystem.Get(actor)
#endif
UPDATABLE_SYSTEM(ZSQD)
};
#endif
