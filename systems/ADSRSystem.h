#pragma once

#include "base/MathUtil.h"

#include "System.h"

struct ADSRComponent {
	bool active;

	float value;
	float activationTime;

	float idleValue;
	float attackValue;
	float attackTiming;
	float sustainValue;
	float decayTiming;
	float releaseTiming;
};
	
#define theADSRSystem ADSRSystem::GetInstance()
#define ADSR(actor) theADSRSystem.Get(actor)

UPDATABLE_SYSTEM(ADSR)
			
};

