#pragma once

#include "System.h"

struct GridComponent {
    enum EType {
        Normal,
    };

	GridComponent(): type(Normal), blocksPath(false), blocksVision(false), canBeOnMultipleCells(false) {}



    EType type;
    bool blocksPath;
    bool blocksVision;
    bool canBeOnMultipleCells;
};

#define theGridSystem GridSystem::GetInstance()
#define GRID(e) theGridSystem.Get(e)

UPDATABLE_SYSTEM(Grid)
};
