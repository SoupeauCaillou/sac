#pragma once

#include "base/Vector2.h"
#include "base/MathUtil.h"

#include "System.h"


struct TransformationComponent {
	TransformationComponent(): position(Vector2::Zero), rotation(0), z(0), parent(0) {
	}
	Vector2 position, worldPosition;
	float rotation, worldRotation;
	float z;

	Entity parent;
};

#define theTransformationSystem TransformationSystem::GetInstance()
#define TRANSFORM(e) theTransformationSystem.Get(e)

UPDATABLE_SYSTEM(Transformation)
};
