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

    componentSerializer.add(new Property<bool>(Murmur::Hash("active"), OFFSET(active, a)));
    componentSerializer.add(new Property<float>(Murmur::Hash("value"), OFFSET(value, a), 0.001f));
    componentSerializer.add(new Property<float>(Murmur::Hash("idle_value"), OFFSET(idleValue, a), 0.001f));
    componentSerializer.add(new Property<float>(Murmur::Hash("attack_value"), OFFSET(attackValue, a), 0.001f));
    componentSerializer.add(new Property<float>(Murmur::Hash("attack_timing"), OFFSET(attackTiming, a), 0.001f));
    componentSerializer.add(new Property<float>(Murmur::Hash("decay_timing"), OFFSET(decayTiming, a), 0.001f));
    componentSerializer.add(new Property<float>(Murmur::Hash("sustain_value"), OFFSET(sustainValue, a), 0.001f));
    componentSerializer.add(new Property<float>(Murmur::Hash("release_timing"), OFFSET(releaseTiming, a), 0.001f));
    componentSerializer.add(new Property<int>(Murmur::Hash("attack_mode"), OFFSET(attackMode, a)));
    componentSerializer.add(new Property<int>(Murmur::Hash("decay_mode"), OFFSET(decayMode, a)));
    componentSerializer.add(new Property<int>(Murmur::Hash("release_mode"), OFFSET(releaseMode, a)));
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
