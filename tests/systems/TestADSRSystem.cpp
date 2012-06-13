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
#include <UnitTest++.h>
#include "systems/ADSRSystem.h"

TEST(SimpleFloatADSR)
{
	ADSRSystem::CreateInstance();
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
	ADSRSystem::DestroyInstance();
}
