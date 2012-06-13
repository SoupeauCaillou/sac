/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
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
	        std::cout << a << ":"<< sc->direction << std::endl;
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
	}
	elements[e] = se;
}