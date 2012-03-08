#pragma once

#include "base/MathUtil.h"

#include "System.h"

enum Mode {
	Linear, 
	Quadratic
};


struct ADSRComponent {
	//ADSRComponent() : attackMode(Linear) {}
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

