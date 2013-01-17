/*!
 * \file ScrollingSystem.h
 * \brief 
 * \author Pierre-Eric Pelloux-Prayer
 * \author Gautier Pelloux-Prayer
 */
 #pragma once

#include "System.h"
#include "RenderingSystem.h"

/*! \struct ScrollingComponent
 *  \brief ? */
struct ScrollingComponent {
	ScrollingComponent() : opaqueType(RenderingComponent::NON_OPAQUE) {}
    std::vector<std::string> images; //!< ?
    Vector2 direction;  //!< 
    float speed; //!< 
    Vector2 displaySize; //!<
    // transitive rendering properties
    bool hide; //!<
    RenderingComponent::Opacity opaqueType; //!<
    float opaqueSeparation; //!<
};

#define theScrollingSystem ScrollingSystem::GetInstance()
#define SCROLLING(actor) theScrollingSystem.Get(actor)

/*! \class Scrolling
 *  \brief ? */
UPDATABLE_SYSTEM(Scrolling)

private:
/*! \struct ScrollingElement
 *  \brief ? */
struct ScrollingElement {
	Entity e[2]; //!< ?
	int imageIndex[2]; //!< ?
	bool hasBeenVisible[2]; //!< ?
};
/*! \brief initialization of something
 * \param e : ?
 * \param sc : ? */
void initScrolling(Entity e, ScrollingComponent* sc);
std::map<Entity, ScrollingElement> elements; //!<
/*! \typedef EltIt
 *  \brief ? */
typedef std::map<Entity, ScrollingElement>::iterator EltIt;

};

