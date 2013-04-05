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

}

void CollisionSystem::DoUpdate(float) {
#if 0
    struct CollisionInfo {
    
    };
    FOR_EACH_ENTITY_COMPONENT(Collision, entity, collc)
        const TransformationComponent* tc = TRANSFORM(entity);
        // find all potential collisions
        for (auto& entry: collc->collisionCallback) {
            TransformationComponent* pltfTC = TRANSFORM(entry->first);
            if (IntersectionUtil::lineLine(
                collc->previousPosition, newPosition,
                    pltfTC->worldPosition + Vector2::Rotate(Vector2(pltfTC->size.X * 0.5, pltfTC->size.Y * 0.5), pltfTC->worldRotation),
                    pltfTC->worldPosition + Vector2::Rotate(Vector2(-pltfTC->size.X * 0.5, pltfTC->size.Y * 0.5), pltfTC->worldRotation),
                    0)) {
        }
    
    }
#endif
}


#if SAC_INGAME_EDITORS
void CollisionSystem::addEntityPropertiesToBar(Entity entity, TwBar*) {
    CollisionComponent* tc = Get(entity, false);
    if (!tc) return;
}
#endif
