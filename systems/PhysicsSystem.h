#pragma once

#include "System.h"

struct Force {
    Vector2 vector;
    Vector2 point;

    Force(const Vector2& v, const Vector2& p) : vector(v), point(p) {}
    Force() {}
};

struct PhysicsComponent {
    Vector2 linearVelocity;
    float angularVelocity;
    float mass;
    Vector2 gravity;
    std::vector<std::pair<Force, float> > forces;
    float momentOfInertia;
};

#define thePhysicsSystem PhysicsSystem::GetInstance()
#define PHYSICS(actor) thePhysicsSystem.Get(actor)
UPDATABLE_SYSTEM(Physics)

public:
	static void addMoment(PhysicsComponent* pc, float m);
};

