/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
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

void ContainerSystem::DoUpdate(float dt __attribute__((unused))) {
	if (components.empty())
		return;

	bool atLeastOneEnabled = false;
	for(ComponentIt it=components.begin(); !atLeastOneEnabled && it!=components.end(); ++it) {
		ContainerComponent* bc = (*it).second;
		atLeastOneEnabled |= (bc->enable && !bc->entities.empty());
	}
		
	if (!atLeastOneEnabled)
		return;
		
	const std::vector<Entity> allEntities(theTransformationSystem.RetrieveAllEntityWithComponent());

	for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
		Entity a = (*it).first;			
		ContainerComponent* bc = (*it).second;

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

