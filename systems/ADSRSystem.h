/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "base/MathUtil.h"

#include "System.h"

enum Mode {
	Linear = 0, 
	Quadratic
};


struct ADSRComponent {
	ADSRComponent() : active(false), value(0), activationTime(0), attackMode(Linear), decayMode(Linear), releaseMode(Linear) {}
	bool active;

	float value;
	float activationTime;

	float idleValue;
	float attackValue;
	float attackTiming;
	float sustainValue;
	float decayTiming;
	float releaseTiming;
	Mode attackMode, decayMode, releaseMode;
};
	
#define theADSRSystem ADSRSystem::GetInstance()
#define ADSR(entity) theADSRSystem.Get(entity)

UPDATABLE_SYSTEM(ADSR)
			
};

