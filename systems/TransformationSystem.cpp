#include "TransformationSystem.h"

INSTANCE_IMPL(TransformationSystem);

TransformationSystem::TransformationSystem() : ComponentSystemImpl<TransformationComponent>("Transformation") {
    TransformationComponent tc;
    componentSerializer.add(new EntityProperty(OFFSET(parent, tc)));
    componentSerializer.add(new EpsilonProperty<float>(OFFSET(position.X, tc), 0.001));
    componentSerializer.add(new EpsilonProperty<float>(OFFSET(position.Y, tc), 0.001));
    componentSerializer.add(new EpsilonProperty<float>(OFFSET(size.X, tc), 0.001));
    componentSerializer.add(new EpsilonProperty<float>(OFFSET(size.Y, tc), 0.001));
    componentSerializer.add(new EpsilonProperty<float>(OFFSET(rotation, tc), 0.001));
    componentSerializer.add(new EpsilonProperty<float>(OFFSET(z, tc), 0.001));
}

void TransformationSystem::DoUpdate(float) {
	//update orphans first
    FOR_EACH_COMPONENT(Transformation, bc)
		if (!bc->parent) {
			bc->worldPosition = bc->position;
			bc->worldRotation = bc->rotation;
            bc->worldZ = bc->z;
		}
	}
	//copy parent property to its sons
    FOR_EACH_COMPONENT(Transformation, bc)
		Entity parent = bc->parent;
		if (parent) {
			while (TRANSFORM(parent)->parent) {
				parent = TRANSFORM(parent)->parent;
			}
			const TransformationComponent* pbc = TRANSFORM(bc->parent);
			bc->worldPosition = pbc->worldPosition + Vector2::Rotate(bc->position, pbc->worldRotation);
			bc->worldRotation = pbc->worldRotation + bc->rotation;
            bc->worldZ = pbc->worldZ + bc->z;
		}
    }
}

void TransformationSystem::setPosition(TransformationComponent* tc, const Vector2& p, PositionReference ref) {
	// x
	switch (ref) {
		case NW:
		case W:
		case SW:
			tc->position.X = p.X + tc->size.X * 0.5;
			break;
		case N:
		case C:
		case S:
			tc->position.X = p.X;
			break;
		case NE:
		case E:
		case SE:
			tc->position.X = p.X - tc->size.X * 0.5;
			break;
	}
	// y
	switch (ref) {
		case NW:
		case N:
		case NE:
			tc->position.Y = p.Y - tc->size.Y * 0.5;
			break;
		case W:
		case C:
		case E:
			tc->position.Y = p.Y;
			break;
		case SW:
		case S:
		case SE:
			tc->position.Y = p.Y + tc->size.Y * 0.5;
			break;
	}
}
