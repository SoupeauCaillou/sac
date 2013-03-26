#pragma once

#include "System.h"

struct Force {
    glm::vec2 vector;
    glm::vec2 point;

    Force(const glm::vec2& v, const glm::vec2& p) : vector(v), point(p) {}
    Force() {}
};

struct PhysicsComponent {
    PhysicsComponent() : linearVelocity(0.0f, 0.0f), angularVelocity(0.0f), mass(0.0f), gravity(0.0f, 0.0f) {}

    glm::vec2 linearVelocity;
    float angularVelocity;
    float mass;
    glm::vec2 gravity;
    std::vector<std::pair<Force, float> > forces;
    float momentOfInertia;
};

#define thePhysicsSystem PhysicsSystem::GetInstance()
#define PHYSICS(actor) thePhysicsSystem.Get(actor)
UPDATABLE_SYSTEM(Physics)

public:
	static void addMoment(PhysicsComponent* pc, float m);
};
