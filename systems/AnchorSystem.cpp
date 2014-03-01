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
#include <set>
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
    bool operator() (const std::pair<Entity, AnchorComponent*>& t1, const std::pair<Entity, AnchorComponent*>& t2) const {
        // if both have the same parent/none, update order doesn't matter
        if (t1.second->parent == t2.second->parent) {
            return (t1.first < t2.first);
        }
        // if they both have parents
        else if (t1.second->parent && t2.second->parent) {
            const auto p1 = theAnchorSystem.Get(t1.second->parent, false);
            const auto p2 = theAnchorSystem.Get(t2.second->parent, false);
            // p1 or p2 may be null
            if (p1 && p2)
                return operator()
                    (std::make_pair(t1.second->parent, p1), std::make_pair(t2.second->parent, p2));
            else if (p1)
                return false;
            else if (p2)
                return true;
            else
                return (t1.first < t2.first);
        }
        // if only t1 has a parent, update it last
        else if (t1.second->parent) {
            return false;
        }
        // else, only t2 has a parent -> t1 first
        else {
            return true;
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
    std::set<std::pair<Entity, AnchorComponent*> , CompareParentChain> cp;

    // sort all, root node first
    std::for_each(components.begin(), components.end(), [&cp] (std::pair<Entity, AnchorComponent*> a) -> void {
        cp.insert(a);
    });

    for (auto p: cp) {
        const auto anchor = p.second;
        if (anchor->parent) {
            const auto pTc = TRANSFORM(anchor->parent);
            auto tc = TRANSFORM(p.first);
            adjustTransformWithAnchor(tc, pTc, anchor);
        }
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
