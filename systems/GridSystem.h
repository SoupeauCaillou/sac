#pragma once

#include "System.h"


struct GridComponent {
	GridComponent(): blocksVision(false), blocksPath(false) {}

	bool blocksVision, blocksPath;

};

#define theGridSystem GridSystem::GetInstance()
#define GRID(e) theGridSystem.Get(e)

UPDATABLE_SYSTEM(Grid)
};
