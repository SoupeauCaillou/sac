#include "ScrollingSystem.h"
#include "base/EntityManager.h"
#include "TransformationSystem.h"
#include "RenderingSystem.h"

INSTANCE_IMPL(ScrollingSystem);
 
ScrollingSystem::ScrollingSystem() : ComponentSystemImpl<ScrollingComponent>("scrolling") {
 
}

void ScrollingSystem::DoUpdate(float dt) {
    for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
        Entity a = (*it).first;
        ScrollingComponent* sc = (*it).second;
        
        EltIt iter = elements.find(a);
        if (iter == elements.end()) {
	        initScrolling(a, sc);
	        iter = elements.find(a);
        }
        
        if (sc->speed != Vector2::Zero) {
	        ScrollingElement& se = iter->second;
	        for (int i=0; i<2; i++) {
		        TransformationComponent* tc = TRANSFORM(se.e[i]);
		        tc->position += sc->speed * dt;
		        bool isVisible = theRenderingSystem.isVisible(tc);
		        if (!se.hasBeenVisible[i] && isVisible) {
			        se.hasBeenVisible[i] = true;
		        } else if (se.hasBeenVisible[i] && !isVisible) {
		        	TransformationComponent* ptc = TRANSFORM(a);
		        	Vector2 normS = -Vector2::Normalize(sc->speed);
			    	se.imageIndex[i] = (se.imageIndex[i] + 2) % sc->images.size();  
			    	RENDERING(se.e[i])->texture = theRenderingSystem.loadTextureFile(sc->images[se.imageIndex[i]]);
			    	tc->position = Vector2(normS.X * ptc->size.X, normS.Y * ptc->size.Y);
			    	se.hasBeenVisible[i] = false;
		        }
	        }
        }
    }
}

void ScrollingSystem::initScrolling(Entity e, ScrollingComponent* sc) {
	ScrollingElement se;
	Vector2 normS = -Vector2::Normalize(sc->speed);
	TransformationComponent* ptc = TRANSFORM(e);
	for (int i=0; i<2; i++) {
		se.e[i] = theEntityManager.CreateEntity();
		ADD_COMPONENT(se.e[i], Transformation);
		ADD_COMPONENT(se.e[i], Rendering);
		
		TransformationComponent* tc = TRANSFORM(se.e[i]);
		tc->parent = e;
		tc->size = sc->displaySize;//
		tc->position = Vector2(normS.X * ptc->size.X, normS.Y * ptc->size.Y) * i;
		tc->z = ptc->z;
		
		RenderingComponent* rc = RENDERING(se.e[i]);
		rc->hide = false;
		se.imageIndex[i] = i % sc->images.size();
		rc->texture = theRenderingSystem.loadTextureFile(sc->images[se.imageIndex[i]]);
		// rc->color = Color(0, 1 - i, 1, 1);
		se.hasBeenVisible[i] = false;
	}
	elements[e] = se;
}