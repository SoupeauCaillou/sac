#pragma once

#include "System.h"

struct GridComponent {
    enum EType {
        Normal,
        House,
        Bush,
        Soldier,
        SoldierDead,
    };

	GridComponent(): type(Normal) {}

    bool blocksVision() { return type == House || type == Bush; }
    bool blocksPath() { return type == House || type == Soldier; }
    bool canBeOnMultipleCells() { return type == House || type == Bush; }

    EType type;
};

#define theGridSystem GridSystem::GetInstance()
#define GRID(e) theGridSystem.Get(e)

UPDATABLE_SYSTEM(Grid)
};
