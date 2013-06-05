#pragma once

#include "System.h"


struct GridComponent {
	GridComponent(): blocksVision(false), blocksPath(false), canBeOnMultipleCells(false) {}

	bool blocksVision, blocksPath;
    bool canBeOnMultipleCells;
};

#define theGridSystem GridSystem::GetInstance()
#define GRID(e) theGridSystem.Get(e)

UPDATABLE_SYSTEM(Grid)
};
