#pragma once

#include "base/MathUtil.h"
#include "base/Log.h"

#include "System.h"
#include <vector>

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
    std::vector<Force> forces;
    float momentOfInertia;
};

#define thePhysicsSystem PhysicsSystem::GetInstance()
#define PHYSICS(actor) thePhysicsSystem.Get(actor)
UPDATABLE_SYSTEM(Physics)

public:
	static void addMoment(PhysicsComponent* pc, float m);
};

