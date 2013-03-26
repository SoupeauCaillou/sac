#include "ContainerSystem.h"
#include "base/MathUtil.h"
#include "TransformationSystem.h"

INSTANCE_IMPL(ContainerSystem);

ContainerSystem::ContainerSystem() : ComponentSystemImpl<ContainerComponent>("Container") {
    /* nothing saved */
}

static void updateMinMax(float& minX, float& minY, float& maxX, float& maxY, TransformationComponent* tc) {
	if (tc->size == Vector2::Zero)
		return;
	minX = MathUtil::Min(minX, tc->worldPosition.X - tc->size.X * 0.5f);
	minY = MathUtil::Min(minY, tc->worldPosition.Y - tc->size.Y * 0.5f);
	maxX = MathUtil::Max(maxX, tc->worldPosition.X + tc->size.X * 0.5f);
	maxY = MathUtil::Max(maxY, tc->worldPosition.Y + tc->size.Y * 0.5f);
}

void ContainerSystem::DoUpdate(float) {
	if (components.empty())
		return;

	bool atLeastOneEnabled = false;
    FOR_EACH_COMPONENT(Container, bc)
		atLeastOneEnabled |= (bc->enable && !bc->entities.empty());
        if (atLeastOneEnabled)
            break;
	}

	if (!atLeastOneEnabled)
		return;

	const std::vector<Entity> allEntities(theTransformationSystem.RetrieveAllEntityWithComponent());

	FOR_EACH_ENTITY_COMPONENT(Container, a, bc)
		if (!bc->enable || bc->entities.empty())
			continue;

		float minX = MathUtil::MaxFloat, minY = MathUtil::MaxFloat;
		float maxX = MathUtil::MinFloat, maxY = MathUtil::MinFloat;
		for(std::vector<Entity>::iterator jt=bc->entities.begin(); jt!=bc->entities.end(); ++jt) {
			TransformationComponent* tc = TRANSFORM(*jt);
			updateMinMax(minX, minY, maxX, maxY, tc);

			if (bc->includeChildren) {
				// arg
				for(std::vector<Entity>::const_iterator kt=allEntities.begin(); kt!=allEntities.end(); ++kt) {
					TransformationComponent* ttc = TRANSFORM(*kt);
					if (ttc->parent == *jt) {
						updateMinMax(minX, minY, maxX, maxY, ttc);
					}
				}
			}
		}

		TransformationComponent* tc = TRANSFORM(a);
		tc->position = Vector2((minX + maxX) * 0.5, (minY + maxY) * 0.5);
		tc->size = Vector2(maxX - minX, maxY - minY);
	}
}

#ifdef SAC_INGAME_EDITORS
void ContainerSystem::addEntityPropertiesToBar(Entity, TwBar*) {

}
#endif