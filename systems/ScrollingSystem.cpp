#include "ScrollingSystem.h"
#include "TransformationSystem.h"
#include "RenderingSystem.h"

#include "base/EntityManager.h"
#include "base/MathUtil.h"

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
	        if (MathUtil::Abs(sc->direction.Length() - 1) <= 0.001) {
	        	initScrolling(a, sc);
	        	iter = elements.find(a);
	        }
	        continue;
        }
        if (sc->hide) {
	        ScrollingElement& se = iter->second;
	        for (int i=0; i<2; i++) {
	        	RENDERING(se.e[i])->hide = true;
	        }
	        continue;
        }

        assert (sc->speed >= 0);
	    ScrollingElement& se = iter->second;
	    for (int i=0; i<2; i++) {
		    if (RENDERING(se.e[i])->hide)
		    	RENDERING(se.e[i])->hide = false;

	        TransformationComponent* tc = TRANSFORM(se.e[i]);
	        TransformationComponent* ptc = TRANSFORM(a);
	        tc->position += sc->direction * (sc->speed * dt);
	        bool isVisible = theRenderingSystem.isVisible(tc);
	        if (!se.hasBeenVisible[i] && isVisible) {
		        se.hasBeenVisible[i] = true;
	        } else if (se.hasBeenVisible[i] && !isVisible) {
	        	se.imageIndex[i] = (se.imageIndex[i] + 2) % sc->images.size();
		    	RENDERING(se.e[i])->texture = theRenderingSystem.loadTextureFile(sc->images[se.imageIndex[i]]);
		    	tc->position = TRANSFORM(se.e[(i+1)%2])->position - Vector2(sc->direction.X * ptc->size.X, sc->direction.Y * ptc->size.Y);
                tc->z = ptc->z - 0.005;
                se.hasBeenVisible[i] = false;
	        }
        }
    }
}

void ScrollingSystem::initScrolling(Entity e, ScrollingComponent* sc) {
	ScrollingElement se;

	assert (MathUtil::Abs(sc->direction.Length() - 1) <= 0.001);

	TransformationComponent* ptc = TRANSFORM(e);
	for (int i=0; i<2; i++) {
		se.e[i] = theEntityManager.CreateEntity();
		ADD_COMPONENT(se.e[i], Transformation);
		ADD_COMPONENT(se.e[i], Rendering);

		TransformationComponent* tc = TRANSFORM(se.e[i]);
		tc->parent = e;
		tc->size = sc->displaySize;
		tc->position = -Vector2(sc->direction.X * ptc->size.X, sc->direction.Y * ptc->size.Y) * i;
		tc->z = ptc->z + i * 0.05;

		RenderingComponent* rc = RENDERING(se.e[i]);
		// rc->hide = false;
		se.imageIndex[i] = i % sc->images.size();
		rc->texture = theRenderingSystem.loadTextureFile(sc->images[se.imageIndex[i]]);
		// rc->color = debugColors[se.imageIndex[i]];
		se.hasBeenVisible[i] = false;
		rc->opaqueType = sc->opaqueType;
	}
	elements[e] = se;
}

#ifdef INGAME_EDITORS
void ScrollingSystem::addEntityPropertiesToBar(Entity, TwBar*) {

}
#endif