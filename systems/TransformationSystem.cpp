#include "TransformationSystem.h"

INSTANCE_IMPL(TransformationSystem);

TransformationSystem::TransformationSystem() : ComponentSystemImpl<TransformationComponent>("Transformation") {
    TransformationComponent tc;
    componentSerializer.add(new EntityProperty(OFFSET(parent, tc)));
    componentSerializer.add(new Property<Vector2>(OFFSET(position, tc), Vector2(0.001, 0)));
    componentSerializer.add(new Property<Vector2>(OFFSET(size, tc), Vector2(0.001, 0)));
    componentSerializer.add(new Property<float>(OFFSET(rotation, tc), 0.001));
    componentSerializer.add(new Property<float>(OFFSET(z, tc), 0.001));
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
    FOR_EACH_ENTITY_COMPONENT(Transformation, entity, bc)
		if (bc->parent) {
			Entity child = entity;
			Entity parent = bc->parent;
			do {
			// while (TRANSFORM(parent)->parent) {
				TransformationComponent* ptc = Get(parent, false);
				if (!ptc) {
					LOG(FATAL) << "Entity '" << theEntityManager.entityName(child) << "' s parent '" << theEntityManager.entityName(parent) << "' has not Transform";
				}
                Entity p = TRANSFORM(parent)->parent;
                #ifdef DEBUG
                LOG_IF(FATAL, p == parent) << "Entity '" << theEntityManager.entityName(parent) << "' parent is itself";
                #endif
                child = parent;
                parent = p;
			} while (parent);
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

#ifdef INGAME_EDITORS
void TransformationSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    TransformationComponent* tc = Get(entity, false);
    if (!tc) return;
    TwAddVarRW(bar, "size.X", TW_TYPE_FLOAT, &tc->size.X, "group=Transformation precision=3 step=0,01");
    TwAddVarRW(bar, "size.Y", TW_TYPE_FLOAT, &tc->size.Y, "group=Transformation precision=3 step=0,01"); 
    TwAddVarRW(bar, "position.X", TW_TYPE_FLOAT, &tc->position.X, "group=local precision=3 step=0,01");
    TwAddVarRW(bar, "position.Y", TW_TYPE_FLOAT, &tc->position.Y, "group=local precision=3 step=0,01"); 
    TwAddVarRW(bar, "rotation", TW_TYPE_FLOAT, &tc->rotation, "group=local step=0,01 precision=3");
    TwAddVarRW(bar, "Z", TW_TYPE_FLOAT, &tc->z, "group=local precision=3 step=0,01");
    TwAddVarRO(bar, "_position.X", TW_TYPE_FLOAT, &tc->worldPosition.X, "group=world precision=3");
    TwAddVarRO(bar, "_position.Y", TW_TYPE_FLOAT, &tc->worldPosition.Y, "group=world precision=3"); 
    TwAddVarRO(bar, "_rotation", TW_TYPE_FLOAT, &tc->worldRotation, "group=world step=0,05 precision=3");
    TwAddVarRO(bar, "_Z", TW_TYPE_FLOAT, &tc->worldZ, "group=world precision=3");
    std::stringstream groups;
    groups << TwGetBarName(bar) << '/' << "local group=Transformation\t\n" << TwGetBarName(bar) << '/' << "world group=Transformation";
    TwDefine(groups.str().c_str());
}
#endif
