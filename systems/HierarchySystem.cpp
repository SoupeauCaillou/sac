#include "HierarchySystem.h"

INSTANCE_IMPL(HierarchySystem);
	
HierarchySystem::HierarchySystem() : ComponentSystem<HierarchyComponent>("Hierarchy") { 
	
}

void HierarchySystem::DoUpdate(float dt) {
	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;			
		HierarchyComponent* bc = (*it).second;

		Entity parent = bc->parent;

		if (parent) {
			/*a->SetPosition(parent->GetPosition() + 
				Vector2::Rotate(bc->localPosition, MathUtil::ToRadians(parent->GetRotation())));
			a->SetLayer(parent->GetLayer() + bc->zOffset);
			a->SetRotation(MathUtil::ToDegrees(bc->localRotation) + parent->GetRotation());*/
		}
	}
}

