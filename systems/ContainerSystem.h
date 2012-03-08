#pragma once

#include "System.h"


struct ContainerComponent {
	std::vector<Entity> entities;
	bool includeChildren;
};

#define theContainerSystem ContainerSystem::GetInstance()
#define CONTAINER(e) theContainerSystem.Get(e)

UPDATABLE_SYSTEM(Container)
};
