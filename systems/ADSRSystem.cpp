#include "ADSRSystem.h"

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

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
    // componentSerializer.add("arg", new Property<ADSRComponent>(0));
}

void ADSRSystem::DoUpdate(float dt) {
    FOR_EACH_COMPONENT(ADSR, adsr)
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
				adsr->value = glm::lerp(adsr->idleValue, adsr->attackValue, adsr->activationTime / adsr->attackTiming);

				} else if (adsr->attackMode == Quadratic) {
					float z = adsr->activationTime / adsr->attackTiming;
					adsr->value = adsr->idleValue + z * z * (adsr->attackValue - adsr->idleValue);
				}

			// phase D
			} else if (adsr->activationTime < (adsr->attackTiming + adsr->decayTiming)) {
				if (adsr->decayMode == Linear) {
				adsr->value = glm::lerp(adsr->attackValue, adsr->sustainValue, (adsr->activationTime - adsr->attackTiming) / adsr->decayTiming);
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
			adsr->activationTime = glm::min(adsr->activationTime, adsr->releaseTiming);
			adsr->activationTime -= dt;
			if (adsr->releaseMode == Linear) {
			    adsr->value = glm::lerp(adsr->idleValue, adsr->sustainValue, adsr->activationTime / adsr->releaseTiming);
			} else if (adsr->releaseMode == Quadratic) {
					float z = (adsr->activationTime-adsr->attackTiming-adsr->decayTiming) / adsr->releaseTiming;
					adsr->value = adsr->sustainValue + z * z * (adsr->idleValue - adsr->sustainValue);
			}
		}
	}
}
