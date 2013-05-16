#pragma once

#include <glm/glm.hpp>

#include "System.h"


struct TransformationComponent {
	TransformationComponent(): position(0.0f), size(1.0f), rotation(0), z(0) {}

	glm::vec2 position, size;
	float rotation;
	float z;
};

#define theTransformationSystem TransformationSystem::GetInstance()
#define TRANSFORM(e) theTransformationSystem.Get(e)

UPDATABLE_SYSTEM(Transformation)
};
