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
#include "ADSRSystem.h"
/*
 *        /\
 * 		 /  \ D
 * 	 A 	/    \    S
 * 	   /      \________
 * 	  /		           \ R
 *   /__________________\
 *	0 
*/
INSTANCE_IMPL(ADSRSystem);

ADSRSystem::ADSRSystem() : ComponentSystemImpl<ADSRComponent>("ADSR") {
    ADSRComponent a;
    componentSerializer.add(new Property(0, sizeof(a)));
}

void ADSRSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(ADSR, entity, adsr)
		if (!adsr->active && adsr->activationTime <= 0) {
			adsr->value = adsr->idleValue;
			adsr->activationTime = 0;
			continue;
		}

		if (adsr->active) {
			adsr->activationTime += dt;
			// phase A
			if (adsr->activationTime < adsr->attackTiming) {
				if (adsr->attackMode == Linear) {
				adsr->value = MathUtil::Lerp(
					adsr->idleValue,
					adsr->attackValue,
					adsr->activationTime / adsr->attackTiming);
				} else if (adsr->attackMode == Quadratic) {
					float z = adsr->activationTime / adsr->attackTiming;
					adsr->value = adsr->idleValue + z * z * (adsr->attackValue - adsr->idleValue); 
				}

			// phase D
			} else if (adsr->activationTime < (adsr->attackTiming + adsr->decayTiming)) {
				if (adsr->decayTiming && adsr->attackValue < adsr->sustainValue) {
					LOGW("Entity '%lu' -> ADSR decay timing %.2f used with attack value %.2f < sustain value %.2f", entity, adsr->decayTiming, adsr->attackValue, adsr->sustainValue);
				}
				if (adsr->decayMode == Linear) {
				adsr->value = MathUtil::Lerp(
					adsr->attackValue,
					adsr->sustainValue,
					(adsr->activationTime - adsr->attackTiming) / adsr->decayTiming);
				} else if (adsr->decayMode == Quadratic) {
					float z = (adsr->activationTime-adsr->attackTiming) / adsr->decayTiming;
					adsr->value = adsr->attackValue + z * z * (adsr->sustainValue - adsr->attackValue); 
				}

			// phase S
			} else {
				adsr->value = adsr->sustainValue;
			}
		//phase R
		} else {
			adsr->activationTime = MathUtil::Min(adsr->activationTime, adsr->releaseTiming);
			adsr->activationTime -= dt;
			if (adsr->releaseMode == Linear) {
			    adsr->value = MathUtil::Lerp(
					adsr->idleValue,
					adsr->sustainValue,
					adsr->activationTime / adsr->releaseTiming);
			} else if (adsr->releaseMode == Quadratic) {
					float z = (adsr->activationTime-adsr->attackTiming-adsr->decayTiming) / adsr->releaseTiming;
					adsr->value = adsr->sustainValue + z * z * (adsr->idleValue - adsr->sustainValue); 	  
			}
		}
	}
}
