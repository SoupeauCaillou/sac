#include "ScrollingSystem.h"
#include "base/EntityManager.h"
#include "TransformationSystem.h"
#include "RenderingSystem.h"

INSTANCE_IMPL(ScrollingSystem);
 
static Color debugColors[] = {
	Color(0.5, 0.5, 0.5, 1),
	Color(1, 0, 0, 1),
	Color(0, 0, 0.5, 1),
	Color(0, 0.5, 0.5, 1)
};

ScrollingSystem::ScrollingSystem() : ComponentSystemImpl<ScrollingComponent>("Scrolling") {
 
}

void ScrollingSystem::DoUpdate(float dt) {
    for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
        Entity a = (*it).first;
        ScrollingComponent* sc = (*it).second;

		
        EltIt iter = elements.find(a);
        if (iter == elements.end()) {
	        initScrolling(a, sc);
	        iter = elements.find(a);
	        continue;
        }
        if (sc->hide) {
	        ScrollingElement& se = iter->second;
	        for (int i=0; i<2; i++) {
	        	RENDERING(se.e[i])->hide = true;
	        }
	        continue;
        }
        
		/*if (sc->speed.LengthSquared() < 0.0001) {
			continue;
		}*/
        
	    ScrollingElement& se = iter->second;
	    for (int i=0; i<2; i++) {
		    if (RENDERING(se.e[i])->hide)
		    	RENDERING(se.e[i])->hide = false;
		    
	        TransformationComponent* tc = TRANSFORM(se.e[i]);
	        tc->position += sc->speed * dt;
	        bool isVisible = theRenderingSystem.isVisible(tc);
	        if (!se.hasBeenVisible[i] && isVisible) {
		        se.hasBeenVisible[i] = true;
	        } else if (se.hasBeenVisible[i] && !isVisible) {
	        	TransformationComponent* ptc = TRANSFORM(a);
	        	// HACK
	        	Vector2 normS = Vector2(1, 0); //-Vector2::Normalize(sc->speed);
		    	se.imageIndex[i] = (se.imageIndex[i] + 2) % sc->images.size();  
		    	RENDERING(se.e[i])->texture = theRenderingSystem.loadTextureFile(sc->images[se.imageIndex[i]]);
		    	tc->position = TRANSFORM(se.e[(i+1)%2])->position + Vector2(normS.X * ptc->size.X, normS.Y * ptc->size.Y);
                tc->z = ptc->z - 0.005;
                se.hasBeenVisible[i] = false;
		    	//RENDERING(se.e[i])->color = debugColors[se.imageIndex[i]];
	        }
        }
        TransformationComponent* ptc = TRANSFORM(a);
		Vector2 normS = Vector2(1, 0);//-Vector2::Normalize(sc->speed);

        if (TRANSFORM(se.e[0])->position.X > TRANSFORM(se.e[1])->position.X) {
            TRANSFORM(se.e[0])->position.X = TRANSFORM(se.e[1])->position.X + normS.X * ptc->size.X;
        } else {
            TRANSFORM(se.e[1])->position.X = TRANSFORM(se.e[0])->position.X + normS.X * ptc->size.X;
        }
        
        if (MathUtil::Abs(TRANSFORM(se.e[1])->position.X - TRANSFORM(se.e[0])->position.X) < ptc->size.X * 0.9) {
	        LOGE("BUGGY SCROLLING POS: %.2f - %.2f (%.2f)", TRANSFORM(se.e[0])->position.X, TRANSFORM(se.e[1])->position.X, ptc->size.X);
        }
        /*
        std::cout << "Diff: " <<
            MathUtil::Abs(TRANSFORM(se.e[0])->position.X - TRANSFORM(se.e[1])->position.X) <<
            ","<<TRANSFORM(se.e[0])->z << "," << TRANSFORM(se.e[1])->z << "," <<
            ptc->size.X << " * " << normS.X << " / " << TRANSFORM(se.e[0])->size.X << "," << TRANSFORM(se.e[1])->size.X << std::endl;
        */
    }
}

void ScrollingSystem::initScrolling(Entity e, ScrollingComponent* sc) {
	ScrollingElement se;
	Vector2 normS = Vector2(1, 0); // -Vector2::Normalize(sc->speed);
	// TEMP HACK
	TransformationComponent* ptc = TRANSFORM(e);
	for (int i=0; i<2; i++) {
		se.e[i] = theEntityManager.CreateEntity();
		ADD_COMPONENT(se.e[i], Transformation);
		ADD_COMPONENT(se.e[i], Rendering);
		
		TransformationComponent* tc = TRANSFORM(se.e[i]);
		tc->parent = e;
		tc->size = sc->displaySize;//
		tc->position = Vector2(normS.X * ptc->size.X, normS.Y * ptc->size.Y) * i;
		tc->z = ptc->z + i * 0.05;
		
		RenderingComponent* rc = RENDERING(se.e[i]);
		// rc->hide = false;
		se.imageIndex[i] = i % sc->images.size();
		rc->texture = theRenderingSystem.loadTextureFile(sc->images[se.imageIndex[i]]);
		// rc->color = debugColors[se.imageIndex[i]];
		se.hasBeenVisible[i] = false;
	}
	elements[e] = se;
}