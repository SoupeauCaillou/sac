#pragma once

#include "base/MathUtil.h"

#include "System.h"

enum Mode {
	Linear = 0, 
	Quadratic
};


struct ADSRComponent {
	ADSRComponent() : active(false), value(0), activationTime(0), attackMode(Linear) {}
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

