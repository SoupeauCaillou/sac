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

	        TransformationComponent* tc = TRANSFORM(se.e[i]);
	        TransformationComponent* ptc = TRANSFORM(a);
	        tc->position += sc->direction * (sc->speed * dt);

	        bool isVisible = theRenderingSystem.isVisible(tc);
	        if (!se.hasBeenVisible[i] && isVisible) {
		        se.hasBeenVisible[i] = true;
	        } else if (se.hasBeenVisible[i] && !isVisible) {
	        	se.imageIndex[i] = (se.imageIndex[i] + 2) % sc->images.size();
		    	RENDERING(se.e[i])->texture = theRenderingSystem.loadTextureFile(sc->images[se.imageIndex[i]]);
		    	tc->position = TRANSFORM(se.e[(i+1)%2])->position - glm::vec2(sc->direction.x * ptc->size.x, sc->direction.y * ptc->size.y);
                tc->z = ptc->z - 0.005;
                se.hasBeenVisible[i] = false;
	        }
        }
    }
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

		TransformationComponent* tc = TRANSFORM(se.e[i]);
		ANCHOR(se.e[i])->parent = e;
		tc->size = sc->displaySize;
		tc->position = -glm::vec2(sc->direction.x * ptc->size.x, sc->direction.y * ptc->size.y) * (float)i;
		tc->z = i * 0.001;

		RenderingComponent* rc = RENDERING(se.e[i]);
		se.imageIndex[i] = i % sc->images.size();
		rc->texture = theRenderingSystem.loadTextureFile(sc->images[se.imageIndex[i]]);
		// rc->color = debugColors[se.imageIndex[i]];
		se.hasBeenVisible[i] = false;
		rc->opaqueType = sc->opaqueType;
	}
	elements[e] = se;
}
