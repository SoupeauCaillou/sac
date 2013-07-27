#pragma once

#include "System.h"

struct GridComponent {
    enum EType {
        Normal,
        Pit,
        Brush,
        House,
    };

	GridComponent(): type(Normal), moveCost(1), blocksPath(false), blocksVision(false), canBeOnMultipleCells(false) {}

    EType type;
    int moveCost;
    bool blocksPath;
    bool blocksVision;
    bool canBeOnMultipleCells;
};

#define theGridSystem GridSystem::GetInstance()
#define GRID(e) theGridSystem.Get(e)

UPDATABLE_SYSTEM(Grid)
public:
    static int GetVisibilityCost(GridComponent::EType type, int distance);
    static int GetDefenceBonus(GridComponent::EType type);
    static int GetAttackBonus(GridComponent::EType type);
};
