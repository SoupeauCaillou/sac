#pragma once

#include "base/MathUtil.h"

#include "System.h"

struct MorphingComponent {
	MorphingComponent() : active(false), value(0), timing(0), activationTime(0), from(Vector2::Zero), to(Vector2::Zero), pos(Vector2::Zero) {}
	bool active;

	Vector2 from, to, pos;

	float value;
	float activationTime;
	float timing;
};

#define theMorphingSystem MorphingSystem::GetInstance()
#define MORPHING(entity) theMorphingSystem.Get(entity)

UPDATABLE_SYSTEM(Morphing)

};


