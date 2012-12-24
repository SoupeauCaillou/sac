#pragma once

#include "base/Vector2.h"
#include "base/MathUtil.h"

#include "System.h"


struct TransformationComponent {
	TransformationComponent(): position(Vector2::Zero), size(1.0f, 1.0f), rotation(0), z(0), parent(0) { }

	Vector2 position, worldPosition, size;
	float rotation, worldRotation;//radians
	float z, worldZ;

	Entity parent;
};

#define theTransformationSystem TransformationSystem::GetInstance()
#define TRANSFORM(e) theTransformationSystem.Get(e)

UPDATABLE_SYSTEM(Transformation)

public:
	enum PositionReference {
		NW, N, NE,
		W , C, E ,
		SW, S, SE
	};
	static void setPosition(TransformationComponent* tc, const Vector2& p, PositionReference ref=C);

	#ifdef DEBUG
	void preDeletionCheck(Entity e) {
		FOR_EACH_COMPONENT(Transformation, bc)
			if (bc->parent == e) {
				LOGE("deleting an entities which is parent ! (Entity %ld)", e);
			}
		}
	}
	#endif
};
