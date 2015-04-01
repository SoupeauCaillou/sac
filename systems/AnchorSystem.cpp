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
#include "util/SerializerProperty.h"
#include "base/EntityManager.h"

INSTANCE_IMPL(AnchorSystem);

AnchorSystem::AnchorSystem() : ComponentSystemImpl<AnchorComponent>(HASH("Anchor", 0xf220ebf3)) {
    AnchorComponent tc;
    componentSerializer.add(new EntityProperty(HASH("parent", 0x7ae3b713), OFFSET(parent, tc)));
    componentSerializer.add(new Property<glm::vec2>(HASH("position", 0xffab91ef), OFFSET(position, tc), glm::vec2(0.001f, 0)));
    componentSerializer.add(new Property<glm::vec2>(HASH("anchor", 0xcc3b6b46), OFFSET(anchor, tc), glm::vec2(0.001f, 0)));
    componentSerializer.add(new Property<float>(HASH("rotation", 0x18f19e94), OFFSET(rotation, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("z", 0x74729461), OFFSET(z, tc), 0.001f));
}

struct CompareParentChain {
    bool operator() (const std::pair<Entity, AnchorComponent*>& t1, const std::pair<Entity, AnchorComponent*>& t2) const {
        const auto& p1 = t1.second->parent;
        const auto& p2 = t2.second->parent;

        // if both have the same parent/none, update order doesn't matter
        if (p1 == p2) {
            return (t1.first < t2.first);
        }
        // if they both have parents
        else if (p1 && p2) {
            const auto ap1 = theAnchorSystem.Get(p1, false);
            const auto ap2 = theAnchorSystem.Get(p2, false);

            // p1 or p2 may be null
            if (ap1 && ap2)
                return operator()
                    (std::make_pair(p1, ap1), std::make_pair(p2, ap2));
            else if (ap1)
                return false;
            else if (ap2)
                return true;
            else
                return (t1.first < t2.first);
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

static int computeChainLength(AnchorSystem* system, int l, const AnchorComponent* ac) {
    if (!ac->parent)
        return l;
    const auto* p = system->Get(ac->parent, false);
    if (!p)
        return l;
    else
        return computeChainLength(system, l + 1, p);
}

void AnchorSystem::DoUpdate(float) {
    int longestChainLength = 1;
    FOR_EACH_ENTITY_COMPONENT(Anchor, e, comp)
        longestChainLength = std::max(longestChainLength, computeChainLength(this, 0, comp));
    }

    for (int i=0; i<longestChainLength; i++) {
        FOR_EACH_ENTITY_COMPONENT(Anchor, e, anchor)
            if (!anchor->parent) continue;
            const auto pTc = TRANSFORM(anchor->parent);
            auto tc = TRANSFORM(e);
            adjustTransformWithAnchor(tc, pTc, anchor);
        }
    }
}

#if SAC_DEBUG
void AnchorSystem::Delete(Entity e) {
    FOR_EACH_ENTITY_COMPONENT(Anchor, child, bc)
        if (bc->parent == e && strcmp(theEntityManager.entityName(child), "__text_letter")) {
            LOGE("deleting an entity which is parent ! (Entity " << e << "/" << theEntityManager.entityName(e) << " is parent of " << child << '/' << theEntityManager.entityName(child) << ')');
        }
    END_FOR_EACH()
    ComponentSystemImpl<AnchorComponent>::Delete(e);
}
#endif
