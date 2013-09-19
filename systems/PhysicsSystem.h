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

struct Force {
    //direction and amplitude of the force
    glm::vec2 vector;

    //application point of the force. (0, 0) is the entity center
    glm::vec2 point;

    Force(const glm::vec2& v, const glm::vec2& p) : vector(v), point(p) {}
    Force() {}
};

struct PhysicsComponent {
    PhysicsComponent() : linearVelocity(0.0f, 0.0f), angularVelocity(0.0f), mass(0.0f),
    gravity(0.0f, 0.0f), frottement(0.f) {}

    //current velocity
    glm::vec2 linearVelocity;
    //current angular (rotation) velocity
    float angularVelocity;

    //if mass <= 0, system is disabled
    float mass;

    //set it to (0, 0) to disable the gravity
    glm::vec2 gravity;

    //opposed to velocity force, coeff in R
    float frottement;

    //a force must be applied for a fixed duration: good value for "singular" force would be ~= 1/60.f
    void addForce(const Force & f, float duration) { forces.push_back(std::pair<Force, float>(f, duration)); }
    void addForce(const glm::vec2 & vector, const glm::vec2 & point, float duration) { forces.push_back(std::pair<Force, float>(Force(vector, point), duration)); }

    //don't modify this directly, use 'addForce' instead
    std::vector<std::pair<Force, float> > forces;

    //physics stuff related to rotation :-). It depends on the shape of the object
    float momentOfInertia;
};

#define thePhysicsSystem PhysicsSystem::GetInstance()
#define PHYSICS(actor) thePhysicsSystem.Get(actor)
UPDATABLE_SYSTEM(Physics)

public:
	static void addMoment(PhysicsComponent* pc, float m);

#if SAC_DEBUG
private:
    void addDebugOnlyDrawForce(const glm::vec2 & pos, const glm::vec2 & size);
    std::vector<std::pair<Entity,std::vector<glm::vec2>>> drawForceVectors;
    unsigned currentDraw;
    float norm2Max;
#endif
};
