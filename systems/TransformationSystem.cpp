#include "TransformationSystem.h"
#include "../base/MathUtil.h"

INSTANCE_IMPL(TransformationSystem);
	
TransformationSystem::TransformationSystem() : ComponentSystemImpl<TransformationComponent>("transformation") { 
	
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
		} else {
			bc->worldPosition = bc->position;
			bc->worldRotation = bc->rotation;
		}
	}
}

