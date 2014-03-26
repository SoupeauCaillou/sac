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



#include "ScrollingSystem.h"

#include "AnchorSystem.h"
#include "TransformationSystem.h"
#include "RenderingSystem.h"

#include "base/EntityManager.h"
#include <glm/glm.hpp>

INSTANCE_IMPL(ScrollingSystem);

static Color debugColors[] = {
    Color(0.5, 0.5, 0.5, 1),
    Color(1, 0, 0, 1),
    Color(0, 0, 0.5, 1),
    Color(0, 0.5, 0.5, 1)
};

ScrollingSystem::ScrollingSystem() : ComponentSystemImpl<ScrollingComponent>("Scrolling") {
    /* nothing saved */
    ScrollingComponent sc;

    componentSerializer.add(new Property<glm::vec2>("direction", OFFSET(direction, sc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<float>("speed", OFFSET(speed, sc), 0.001f));
    componentSerializer.add(new Property<glm::vec2>("display_size", OFFSET(displaySize, sc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<bool>("show", OFFSET(show, sc)));
    componentSerializer.add(new Property<uint8_t>("rendering_flags", OFFSET(renderingFlags, sc)));
}

void ScrollingSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Scrolling, a, sc)
        EltIt iter = elements.find(a);
        if (iter == elements.end()) {
            if ( glm::abs(glm::length(sc->direction) - 1) <= 0.001) {
                initScrolling(a, sc);
                iter = elements.find(a);
            }
            continue;
        }
        if (!sc->show) {
            ScrollingElement& se = iter->second;
            for (int i=0; i<2; i++) {
                RENDERING(se.e[i])->show = false;
            }
            continue;
        }

        LOGF_IF(sc->speed < 0, "Scrolling component '" << sc << "' has a speed < 0");

        ScrollingElement& se = iter->second;
        for (int i=0; i<2; i++) {
            RENDERING(se.e[i])->show = true;

            AnchorComponent* tc = ANCHOR(se.e[i]);
            tc->position += sc->direction * (sc->speed * dt);

            bool isVisible = theRenderingSystem.isVisible(se.e[i]);
            if (!se.hasBeenVisible[i] && isVisible) {
                se.hasBeenVisible[i] = true;
            } else if (se.hasBeenVisible[i] && !isVisible) {
                se.imageIndex[i] = (se.imageIndex[i] + 2) % sc->images.size();
                RENDERING(se.e[i])->texture = sc->images[se.imageIndex[i]];
                const auto* ptc = TRANSFORM(a);
                tc->position =
                    ANCHOR(se.e[(i+1)%2])->position -
                    glm::vec2(sc->direction.x * ptc->size.x, sc->direction.y * ptc->size.y);
                se.hasBeenVisible[i] = false;
            }
        }
    END_FOR_EACH()
}

void ScrollingSystem::initScrolling(Entity e, ScrollingComponent* sc) {
    ScrollingElement se;

    TransformationComponent* ptc = TRANSFORM(e);
    for (int i=0; i<2; i++) {

#if SAC_DEBUG
        se.e[i] = theEntityManager.CreateEntity("scroll_" + theEntityManager.entityName(e));
#else
        se.e[i] = theEntityManager.CreateEntity("");
#endif

        ADD_COMPONENT(se.e[i], Transformation);
        ADD_COMPONENT(se.e[i], Rendering);
        ADD_COMPONENT(se.e[i], Anchor);

        TRANSFORM(se.e[i])->size = sc->displaySize;
        auto* tc = ANCHOR(se.e[i]);
        tc->parent = e;
        tc->position = -glm::vec2(sc->direction.x * ptc->size.x, sc->direction.y * ptc->size.y) * (float)i;
        tc->z = i * 0.001f;

        RenderingComponent* rc = RENDERING(se.e[i]);
        se.imageIndex[i] = i % sc->images.size();
        rc->texture = sc->images[se.imageIndex[i]];
        // rc->color = debugColors[se.imageIndex[i]];
        se.hasBeenVisible[i] = false;
        rc->flags = sc->renderingFlags;
    }
    elements[e] = se;
}
