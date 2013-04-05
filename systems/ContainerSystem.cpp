#include "ContainerSystem.h"
#include <glm/glm.hpp>
#include "TransformationSystem.h"
#include <limits>

INSTANCE_IMPL(ContainerSystem);

ContainerSystem::ContainerSystem() : ComponentSystemImpl<ContainerComponent>("Container") {
    /* nothing saved */
}

static void updateMinMax(float& minX, float& minY, float& maxX, float& maxY, TransformationComponent* tc) {
	if (tc->size == glm::vec2(0.0f))
		return;
	minX = glm::min(minX, tc->worldPosition.x - tc->size.x * 0.5f);
	minY = glm::min(minY, tc->worldPosition.y - tc->size.y * 0.5f);
	maxX = glm::max(maxX, tc->worldPosition.x + tc->size.x * 0.5f);
	maxY = glm::max(maxY, tc->worldPosition.y + tc->size.y * 0.5f);
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

        //~ TODO
		float minX = std::numeric_limits<float>().max(), minY = std::numeric_limits<float>().max();
		float maxX = std::numeric_limits<float>().min(), maxY = std::numeric_limits<float>().min();
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
		tc->position = glm::vec2((minX + maxX) * 0.5, (minY + maxY) * 0.5);
		tc->size = glm::vec2(maxX - minX, maxY - minY);
	}
}

#if SAC_INGAME_EDITORS
void ContainerSystem::addEntityPropertiesToBar(Entity, TwBar*) {

}
#endif
