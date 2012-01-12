#include <UnitTest++.h>
#include "systems/ADSRSystem.h"

TEST(SimpleFloatADSR)
{
	Entity e = 1;
	theADSRSystem.Add(e);
	ADSRComponent* ac = ADSR(e);
	ac->idleValue = 0.0;
	ac->attackValue = 1.2;
	ac->attackTiming = 1.0;
	ac->sustainValue = 1.0;
	ac->decayTiming = 0.2;
	ac->releaseTiming = 0.5;

	CHECK_CLOSE(0.0, ac->value, 0.0001);
	ac->active = true;
	theADSRSystem.Update(0.1);
	CHECK_CLOSE(0.12, ac->value, 0.0001);
	theADSRSystem.Update(0.9);
	CHECK_CLOSE(1.2, ac->value, 0.0001);
	theADSRSystem.Update(0.2);
	CHECK_CLOSE(1.0, ac->value, 0.0001);
	theADSRSystem.Update(0.5);
	CHECK_CLOSE(1.0, ac->value, 0.0001);
	ac->active = false;
	theADSRSystem.Update(0.25);
	CHECK_CLOSE(0.5, ac->value, 0.0001);
	theADSRSystem.Update(0.25);
	CHECK_CLOSE(0.0, ac->value, 0.0001);
}
