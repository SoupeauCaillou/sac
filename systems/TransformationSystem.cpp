#include "TransformationSystem.h"
#include <set>

INSTANCE_IMPL(TransformationSystem);

TransformationSystem::TransformationSystem() : ComponentSystemImpl<TransformationComponent>("Transformation") {
    TransformationComponent tc;
    componentSerializer.add(new EntityProperty(OFFSET(parent, tc)));
    componentSerializer.add(new Property<Vector2>(OFFSET(position, tc), Vector2(0.001, 0)));
    componentSerializer.add(new Property<Vector2>(OFFSET(size, tc), Vector2(0.001, 0)));
    componentSerializer.add(new Property<float>(OFFSET(rotation, tc), 0.001));
    componentSerializer.add(new Property<float>(OFFSET(z, tc), 0.001));
}


struct CompareParentChain {
    bool operator() (TransformationComponent* t1, TransformationComponent* t2) const {
        if (t1->parent == t2->parent) {
            return (t1 < t2);
        } else if (t1->parent && t2->parent) {
            return operator()(TRANSFORM(t1->parent), TRANSFORM(t2->parent));
        } else if (t1->parent) {
            return false;
        } else {
            return true;
        }
    }
};

void TransformationSystem::DoUpdate(float) {
    std::set<TransformationComponent*, CompareParentChain> cp;

    // sort all, root node first
    FOR_EACH_COMPONENT(Transformation, bc)
        cp.insert(bc);
    }

    for (auto trans: cp) {
        if (!trans->parent) {
            trans->worldPosition = trans->position;
            trans->worldRotation = trans->rotation;
            trans->worldZ = trans->z;
        } else {
            const TransformationComponent* pbc = TRANSFORM(trans->parent);
            trans->worldPosition = pbc->worldPosition + Vector2::Rotate(trans->position, pbc->worldRotation);
            trans->worldRotation = pbc->worldRotation + trans->rotation;
            trans->worldZ = pbc->worldZ + trans->z;
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

#ifdef SAC_INGAME_EDITORS
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
