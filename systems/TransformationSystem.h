#pragma once

#include "System.h"
#include "../base/Vector2.h"

struct TransformationComponent {
	TransformationComponent(): position(Vector2::Zero), rotation(0), parent(0) {
	}
	Vector2 position, worldPosition;
	float rotation, worldRotation;

	Entity parent;
};

#define theTransformationSystem TransformationSystem::GetInstance()
#define TRANSFORM(e) theTransformationSystem.Get(e)

UPDATABLE_SYSTEM(Transformation)
};
