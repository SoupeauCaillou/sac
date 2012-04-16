#pragma once

#include "base/Vector2.h"
#include "base/MathUtil.h"

#include "System.h"


struct TransformationComponent {
	TransformationComponent(): size(1.0f, 1.0f), position(Vector2::Zero), rotation(0), z(100), parent(0) {
	}
	Vector2 position, worldPosition, size;
	float rotation, worldRotation;
	float z;

	Entity parent;
};

#define theTransformationSystem TransformationSystem::GetInstance()
#define TRANSFORM(e) theTransformationSystem.Get(e)

UPDATABLE_SYSTEM(Transformation)
};
