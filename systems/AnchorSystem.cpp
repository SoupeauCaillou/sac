/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "AnchorSystem.h"

#include "TransformationSystem.h"
#include <forward_list>
#include <glm/gtx/rotate_vector.hpp>

INSTANCE_IMPL(AnchorSystem);

AnchorSystem::AnchorSystem() : ComponentSystemImpl<AnchorComponent>("Anchor") {
    AnchorComponent tc;
    componentSerializer.add(new EntityProperty("parent", OFFSET(parent, tc)));
    componentSerializer.add(new Property<glm::vec2>("position", OFFSET(position, tc), glm::vec2(0.001f, 0)));
    componentSerializer.add(new Property<glm::vec2>("anchor", OFFSET(anchor, tc), glm::vec2(0.001f, 0)));
    componentSerializer.add(new Property<float>("rotation", OFFSET(rotation, tc), 0.001f));
    componentSerializer.add(new Property<float>("z", OFFSET(z, tc), 0.001f));
}

struct CompareParentChain {
#if 0
    bool operator() (const std::pair<Entity, AnchorComponent*>& t1, const std::pair<Entity, AnchorComponent*>& t2) const {
        const Entity e1 = t1.first;
        const Entity e2 = t2.first;
        const auto& p1 = t1.second->parent;
        const auto& p2 = t2.second->parent;
#else

    const std::vector<AnchorComponent>* components;
    CompareParentChain(const std::vector<AnchorComponent>* _components) : components(_components) {}
    bool operator() (Entity e1, Entity e2) const {
        const auto& p1 = (*components)[e1].parent;
        const auto& p2 = (*components)[e2].parent;
#endif

        // if both have the same parent/none, update order doesn't matter
        if (p1 == p2) {
            return (e1 < e2);
        }
        // if they both have parents
        else if (p1 && p2) {
            const auto ap1 = theAnchorSystem.Get(p1, false);
            const auto ap2 = theAnchorSystem.Get(p2, false);

            // p1 or p2 may be null
            if (ap1 && ap2)
                return operator()
#if 0
                    (std::make_pair(p1, ap1), std::make_pair(p2, ap2));
#else
                    (p1, p2);
#endif
            else if (ap1)
                return false;
            else if (ap2)
                return true;
            else
                return (e1 < e2);
        }
        // else parent-less (parent == 0) is < parented one (parent > 0)
        else {
            return p1 < p2;
        }
    }
};

glm::vec2 AnchorSystem::adjustPositionWithAnchor(const glm::vec2& position, const glm::vec2& anchor) {
    return position - anchor;
}


glm::vec2 AnchorSystem::adjustPositionWithCardinal(const glm::vec2& position, const glm::vec2& size, Cardinal::Enum cardinal) {
    const glm::vec2 modifiers[] = {
        glm::vec2(-0.5, 0.5) , glm::vec2(0, 0.5) , glm::vec2(0.5, 0.5),
        glm::vec2(-0.5, 0.0) , glm::vec2(0, 0)   , glm::vec2(0.5, 0),
        glm::vec2(-0.5, -0.5), glm::vec2(0, -0.5), glm::vec2(0.5, -0.5)
    };
    return adjustPositionWithAnchor(position, size * modifiers[(int)cardinal]);
}

void AnchorSystem::adjustTransformWithAnchor(TransformationComponent* tc, const TransformationComponent* parentTc, const AnchorComponent* anchor) {
    // compute global rotation first
    tc->rotation = parentTc->rotation + anchor->rotation;
    // then position
    tc->position = parentTc->position
        + glm::rotate(anchor->position, parentTc->rotation)
        - glm::rotate(anchor->anchor, tc->rotation)
        ;
    // and z
    tc->z = parentTc->z + anchor->z;
}

void AnchorSystem::DoUpdate(float) {
    // insertion sort is a good fit: small data set, almost always sorted
    CompareParentChain cmp(&components);
    const unsigned count = entityWithComponent.size();
    for (unsigned i=1; i<count; i++) {
        int j = i - 1;
        const auto v = entityWithComponent[i];
        // decrease j while elt_i < elt_j
        while (j >= 0 && cmp(v, entityWithComponent[j])) {
            j--;
        }
        // insert at j+1
        if ((j+1) < i) {
            entityWithComponent.erase(entityWithComponent.begin() + i);
            entityWithComponent.emplace(entityWithComponent.begin() + (j+1), v);
        }
    }

    LOGF_IF(entityWithComponent.size() != count, "Insertion sort is broken: " << count << " elements before, " << entityWithComponent.size() << " after");

    for (auto e: entityWithComponent) {
        const auto anchor = &components[e];
        if (!anchor->parent) {
            continue;
        }

        const auto pTc = TRANSFORM(anchor->parent);
        auto tc = TRANSFORM(e);
        adjustTransformWithAnchor(tc, pTc, anchor);
    }
}

#if SAC_DEBUG
void AnchorSystem::Delete(Entity e) {
    FOR_EACH_ENTITY_COMPONENT(Anchor, child, bc)
        if (bc->parent == e && theEntityManager.entityName(child) != "__text_letter") {
            LOGE("deleting an entity which is parent ! (Entity " << e << "/" << theEntityManager.entityName(e) << " is parent of " << child << '/' << theEntityManager.entityName(child) << ')');
        }
    END_FOR_EACH()
    ComponentSystemImpl<AnchorComponent>::Delete(e);
}
#endif
