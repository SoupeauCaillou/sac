#include "TransformationSystem.h"
#include <set>
#include <glm/gtx/rotate_vector.hpp>

INSTANCE_IMPL(TransformationSystem);

TransformationSystem::TransformationSystem() : ComponentSystemImpl<TransformationComponent>("Transformation") {
    TransformationComponent tc;
    componentSerializer.add(new EntityProperty("parent", OFFSET(parent, tc)));
    componentSerializer.add(new Property<glm::vec2>("position", OFFSET(position, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<glm::vec2>("size", OFFSET(size, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<float>("rotation", OFFSET(rotation, tc), 0.001));
    componentSerializer.add(new Property<float>("z", OFFSET(z, tc), 0.001));
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
            trans->worldPosition = pbc->worldPosition + glm::rotate(trans->position, pbc->worldRotation);
            trans->worldRotation = pbc->worldRotation + trans->rotation;
            trans->worldZ = pbc->worldZ + trans->z;
        }
    }
}

void TransformationSystem::setPosition(TransformationComponent* tc, const glm::vec2& p, PositionReference ref) {
	// x
	switch (ref) {
		case NW:
		case W:
		case SW:
			tc->position.x = p.x + tc->size.x * 0.5;
			break;
		case N:
		case C:
		case S:
			tc->position.x = p.x;
			break;
		case NE:
		case E:
		case SE:
			tc->position.x = p.x - tc->size.x * 0.5;
			break;
	}
	// y
	switch (ref) {
		case NW:
		case N:
		case NE:
			tc->position.y = p.y - tc->size.y * 0.5;
			break;
		case W:
		case C:
		case E:
			tc->position.y = p.y;
			break;
		case SW:
		case S:
		case SE:
			tc->position.y = p.y + tc->size.y * 0.5;
			break;
	}
}

void TransformationSystem::Delete(Entity e) {
    #if SAC_DEBUG
    FOR_EACH_ENTITY_COMPONENT(Transformation, child, bc)
        if (bc->parent == e) {
            LOGE("deleting an entity which is parent ! (Entity " << e << "/" << theEntityManager.entityName(e) << " is parent of " << child << '/' << theEntityManager.entityName(child) << ')')
        }
    }
    #endif
    ComponentSystemImpl<TransformationComponent>::Delete(e);
}

#if SAC_INGAME_EDITORS
void TransformationSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    TransformationComponent* tc = Get(entity, false);
    if (!tc) return;
    TwAddVarRW(bar, "size.X", TW_TYPE_FLOAT, &tc->size.x, "group=Transformation precision=3 step=0,01");
    TwAddVarRW(bar, "size.Y", TW_TYPE_FLOAT, &tc->size.y, "group=Transformation precision=3 step=0,01");
    TwAddVarRW(bar, "position.X", TW_TYPE_FLOAT, &tc->position.x, "group=local precision=3 step=0,01");
    TwAddVarRW(bar, "position.Y", TW_TYPE_FLOAT, &tc->position.y, "group=local precision=3 step=0,01");
    TwAddVarRW(bar, "rotation", TW_TYPE_FLOAT, &tc->rotation, "group=local step=0,01 precision=3");
    TwAddVarRW(bar, "Z", TW_TYPE_FLOAT, &tc->z, "group=local precision=3 step=0,01");
    TwAddVarRO(bar, "_position.X", TW_TYPE_FLOAT, &tc->worldPosition.x, "group=world precision=3");
    TwAddVarRO(bar, "_position.Y", TW_TYPE_FLOAT, &tc->worldPosition.y, "group=world precision=3");
    TwAddVarRO(bar, "_rotation", TW_TYPE_FLOAT, &tc->worldRotation, "group=world step=0,05 precision=3");
    TwAddVarRO(bar, "_Z", TW_TYPE_FLOAT, &tc->worldZ, "group=world precision=3");
    std::stringstream groups;
    groups << TwGetBarName(bar) << '/' << "local group=Transformation\t\n" << TwGetBarName(bar) << '/' << "world group=Transformation";
    TwDefine(groups.str().c_str());
}
#endif
