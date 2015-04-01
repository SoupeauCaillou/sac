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
#if !DISABLE_SPOT_SYSTEM
#include "SpotSystem.h"
#include "TransformationSystem.h"
#include "util/IntersectionUtil.h"

#include <glm/gtx/norm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include "util/SerializerProperty.h"

INSTANCE_IMPL(SpotSystem);

SpotSystem::SpotSystem() : ComponentSystemImpl<SpotComponent>(HASH("Spot", 0x5eea6198)) {
    SpotComponent tc;
    componentSerializer.add(new Property<float>(HASH("angle", 0xf644d337), OFFSET(angle, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("distance", 0x271ce505), OFFSET(distance, tc), 0.001f));
    componentSerializer.add(new Property<int>(HASH("resolution", 0xc9d35125), OFFSET(resolution, tc)));
}

struct MyRectangle {
    glm::vec2 position, size;
    float rotation;
};

void SpotSystem::DoUpdate(float) {
    // retrieve all blockers
    const unsigned count = theSpotBlockSystem.entityCount();
    if (count == 0)
        return;
    std::vector<MyRectangle> blocks;
    blocks.reserve(count);
    theSpotBlockSystem.forEachEntityDo([&] (Entity e) -> void {
        const auto* tc = TRANSFORM(e);
        MyRectangle rect;
        rect.position = tc->position;
        rect.size = tc->size;
        rect.rotation = tc->rotation;
        blocks.push_back(rect);
    });

    FOR_EACH_ENTITY_COMPONENT(Spot, e, sc)
        auto* tc = TRANSFORM(e);
        const glm::vec2 p1 = tc->position;

        // clear previous result
        sc->area.vertices.clear();
        sc->area.indices.clear();

        // are defined as a triangle fan
        sc->area.vertices.push_back(p1);

        LOGF_IF(sc->resolution == 0, "Invalid resolution: " << sc->resolution << ". Must be > 0");
        const float angleStep = sc->angle / sc->resolution;
        for (int i=0; i<=sc->resolution; i++) {
            // compute raycast end point
            const glm::vec2 p2 = p1 + glm::rotate(glm::vec2(sc->distance, 0), angleStep * i);

            // raycast
            glm::vec2 nearestIntersection = p2;
            float minDistance2 = glm::distance2(p1, p2);
            glm::vec2 intersections[2];
            for (const auto& rect: blocks) {
                int count =
                    IntersectionUtil::lineRectangle(p1, p2, rect.position, rect.size, rect.rotation, intersections);

                for (int j=0; j<count; j++) {
                    float d = glm::distance2(p1, intersections[j]);
                    if (d < minDistance2) {
                        nearestIntersection = intersections[j];
                        minDistance2 = d;
                    }
                }
            }
            sc->area.vertices.push_back(nearestIntersection);
        }

        // define indices
        int count = sc->area.vertices.size();
        for (int i=0; i<count - 2; i++) {
            sc->area.indices.push_back(0);
            sc->area.indices.push_back(1 + i);
            sc->area.indices.push_back(1 + i + 1);
        }
        LOGF_IF((int)sc->area.indices.size() != sc->resolution * 3,
            "Incorrect number of indices (3 indices par step expected. Resolution=" << sc->resolution << ", indices count: " << sc->area.indices.size());
    }
}

INSTANCE_IMPL(SpotBlockSystem);

SpotBlockSystem::SpotBlockSystem() : ComponentSystemImpl<SpotBlockComponent>(HASH("SpotBlock", 0x5f0d912f)) { }

void SpotBlockSystem::DoUpdate(float) {

}
#endif
