#include "TransformationSystem.h"

INSTANCE_IMPL(TransformationSystem);
	
TransformationSystem::TransformationSystem() : ComponentSystemImpl<TransformationComponent>("Transformation") { 
	
}

void TransformationSystem::DoUpdate(float dt) {
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;			
		TransformationComponent* bc = (*it).second;

		Entity parent = bc->parent;

		if (parent) {
			const TransformationComponent* pbc = TRANSFORM(bc->parent);
			bc->worldPosition = pbc->worldPosition + Vector2::Rotate(bc->position, MathUtil::ToRadians(pbc->worldRotation));
			bc->worldRotation = pbc->worldRotation + bc->rotation;
			bc->z = pbc->z;
		} else {
			bc->worldPosition = bc->position;
			bc->worldRotation = bc->rotation;
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