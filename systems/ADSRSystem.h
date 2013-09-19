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

enum Mode {
	Linear = 0,
	Quadratic
};


struct ADSRComponent {
	ADSRComponent() : value(0), activationTime(0), active(false), idleValue(0.), attackValue(1.), attackTiming(1.), decayTiming(1.),
    sustainValue(2.), releaseTiming(1.), attackMode(Linear), decayMode(Linear), releaseMode(Linear) {}

////// READ ONLY variables
    //Current value. When 'active' is set to true, it start at 'idleValue' and goes to 'attackValue' in 'attackTiming' duration.
    //Then it goes to 'sustainValue' in 'decayTiming' duration.
    //It stays here until 'active' is reset to false, where  it goes back to 'idleValue' in 'releaseTiming' duration.
	float value;
    //represents our position in the phases. When 'active' is set to true, we move forward (trying to go in Sustain mode),
    //and when 'active' is false, we are going back to Release mode
	float activationTime;
////// END OF READ ONLY variables


////// READ/WRITE variables
    // if false, value = idle value, else see above
    bool active;
////// END OF READ/WRITE variables

////// WRITE ONLY variables
	float idleValue;
	float attackValue;
	float attackTiming;
	float decayTiming;
    float sustainValue;
	float releaseTiming;
	Mode attackMode, decayMode, releaseMode;
////// END OF WRITE ONLY variables
};

#define theADSRSystem ADSRSystem::GetInstance()
#define ADSR(entity) theADSRSystem.Get(entity)

UPDATABLE_SYSTEM(ADSR)

};

