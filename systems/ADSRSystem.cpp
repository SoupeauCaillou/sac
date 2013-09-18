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

    componentSerializer.add(new Property<bool>("active", OFFSET(active, a)));
    componentSerializer.add(new Property<float>("value", OFFSET(value, a), 0.001));
    componentSerializer.add(new Property<float>("idle_value", OFFSET(idleValue, a), 0.001));
    componentSerializer.add(new Property<float>("attack_value", OFFSET(attackValue, a), 0.001));
    componentSerializer.add(new Property<float>("attack_timing", OFFSET(attackTiming, a), 0.001));
    componentSerializer.add(new Property<float>("decay_timing", OFFSET(decayTiming, a), 0.001));
    componentSerializer.add(new Property<float>("sustain_value", OFFSET(sustainValue, a), 0.001));
    componentSerializer.add(new Property<float>("release_timing", OFFSET(releaseTiming, a), 0.001));
    componentSerializer.add(new Property<int>("attack_mode", OFFSET(attackMode, a)));
    componentSerializer.add(new Property<int>("decay_mode", OFFSET(decayMode, a)));
    componentSerializer.add(new Property<int>("release_mode", OFFSET(releaseMode, a)));
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
	END_FOR_EACH()
}
