/*!
 * \file MorphingSystem.h
 * \brief 
 * \author Pierre-Eric Pelloux-Prayer
 * \author Gautier Pelloux-Prayer
 */
#pragma once

#include "System.h"

/*! \struct ContainerComponent
 *  \brief */
struct ContainerComponent {
	ContainerComponent() : enable(false), includeChildren(false) {}
	bool enable, includeChildren; //!<
	std::vector<Entity> entities; //!<
};

#define theContainerSystem ContainerSystem::GetInstance()
#define CONTAINER(e) theContainerSystem.Get(e)

/*! \class Container
 *  \brief */
UPDATABLE_SYSTEM(Container)
};
