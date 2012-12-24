#pragma once

#include "System.h"


struct ContainerComponent {
	ContainerComponent() : enable(false), includeChildren(false) {}
	bool enable, includeChildren;
	std::vector<Entity> entities;
};

#define theContainerSystem ContainerSystem::GetInstance()
#define CONTAINER(e) theContainerSystem.Get(e)

UPDATABLE_SYSTEM(Container)
};
