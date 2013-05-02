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
    PhysicsComponent() : linearVelocity(0.0f, 0.0f), angularVelocity(0.0f), mass(0.0f), gravity(0.0f, 0.0f) {}

    //current velocity
    glm::vec2 linearVelocity;
    //current angular (rotation) velocity
    float angularVelocity;

    //if mass <= 0, system is disabled
    float mass;

    //set it to (0, 0) to disable the gravity
    glm::vec2 gravity;

    //a force must be applied for a fixed duration: good value would be 'dt' ~= 1/60.f
    void addForce(const Force & f, float duration) { forces.push_back(std::pair<Force, float>(f, duration)); }

    //don't modify this directly, use 'addForce' instead
    std::vector<std::pair<Force, float> > forces;

    //physics stuff related to rotation :-). It depends of the shape of the object
    float momentOfInertia;
};

#define thePhysicsSystem PhysicsSystem::GetInstance()
#define PHYSICS(actor) thePhysicsSystem.Get(actor)
UPDATABLE_SYSTEM(Physics)

public:
	static void addMoment(PhysicsComponent* pc, float m);
};
