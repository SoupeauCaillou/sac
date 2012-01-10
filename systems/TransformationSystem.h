#pragma once

#include "System.h"
#include "../base/Vector2.h"

struct TransformationComponent {
	TransformationComponent() {
		parent = 0;
	}
	Vector2 position, worldPosition;
	float rotation, worldRotation;

	Entity parent;
};

#define theTransformationSystem TransformationSystem::GetInstance()
#define TRANSFORM(e) theTransformationSystem.Get(e)

UPDATABLE_SYSTEM(Transformation)
};
