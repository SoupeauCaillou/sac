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

#if !DISABLE_AUTONOMOUS_SYSTEM
#include "../steering/SteeringBehavior.h"

#include "System.h"

struct BehaviorParams {
    BehaviorParams() : weight(0.0f), coeff(1.0) {}

    float weight;
    float coeff;
};

struct AutonomousAgentComponent {
    AutonomousAgentComponent() :
        maxSpeed(1.f), maxForce(1.f),
        seekTarget(0), seekParams(),
        fleeTarget(0), fleeParams(), fleeRadius(1.f),
        obstaclesParams(),
        wallsParams(),
        boxParams(),
        wanderParams(),
        cohesionParams(),
        alignementParams() {}

    float maxSpeed, maxForce;
    union {
        Entity seekTarget; // keep speed until target reached
        Entity arriveTarget; // slow down when approching the target
    };
    union {
        BehaviorParams seekParams;
        BehaviorParams arriveParams;
    };
    float arriveDeceleration;

    Entity fleeTarget;
    BehaviorParams fleeParams;
    float fleeRadius;

    std::list<Entity> obstacles;
    BehaviorParams obstaclesParams;

    std::list<Entity> walls;
    BehaviorParams wallsParams;

    glm::vec2 boxPosition, boxSize;
    BehaviorParams boxParams;

    SteeringBehavior::WanderParams wander;
    BehaviorParams wanderParams;

    // Group behaviors
    std::list<Entity> cohesionNeighbors;
    BehaviorParams cohesionParams;

    // Group behaviors
    std::list<Entity> alignementNeighbors;
    BehaviorParams alignementParams;

    // Group behaviors
    std::list<Entity> separationNeighbors;
    BehaviorParams separationParams;
};

#define theAutonomousAgentSystem AutonomousAgentSystem::GetInstance()
#if SAC_DEBUG
#define AUTONOMOUS(entity) theAutonomousAgentSystem.Get(entity,true,__FILE__,__LINE__)
#else
#define AUTONOMOUS(entity) theAutonomousAgentSystem.Get(entity)
#endif

UPDATABLE_SYSTEM(AutonomousAgent)
public:
    static bool isArrived(Entity e);

};
#endif
