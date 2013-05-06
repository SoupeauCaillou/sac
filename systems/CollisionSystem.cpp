/*
    This file is part of sac.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer

    RecursiveRunner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    RecursiveRunner is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with RecursiveRunner.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "CollisionSystem.h"

INSTANCE_IMPL(CollisionSystem);

CollisionSystem::CollisionSystem() : ComponentSystemImpl<CollisionComponent>("Collision") {
    CollisionComponent tc;
    componentSerializer.add(new Property<int>("group", OFFSET(group, tc), 0));
    componentSerializer.add(new Property<int>("collide_with", OFFSET(collideWith, tc), 0));
}

void CollisionSystem::DoUpdate(float) {
    #define CELL_SIZE 1.0f
    struct Cell {
        std::vector<CollisionComponent*> collisions;
        std::vector<TransformationComponent*> transforms;
        int groupsWithin;
    };

    // Assign each entity to a cell
    FOR_EACH_ENTITY_COMPONENT(Collision, entity, cc)
        TransformationComponent* tc = TRANSFORM(entity);
        int cellRow = glm::floor(tc->worldPosition.x / CELL_SIZE);
        int cellCol = glm::floor(tc->worldPosition.y / CELL_SIZE);
    }
}


#if SAC_INGAME_EDITORS
void CollisionSystem::addEntityPropertiesToBar(Entity entity, TwBar*) {
    CollisionComponent* tc = Get(entity, false);
    if (!tc) return;
    TwAddVarRW(bar, "group", TW_TYPE_INT32, &tc->group, "group=Collision");
    TwAddVarRW(bar, "collideWith", TW_TYPE_INT32, &tc->collideWith, "group=Collision");
}
#endif
