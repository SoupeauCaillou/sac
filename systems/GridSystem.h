/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



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
