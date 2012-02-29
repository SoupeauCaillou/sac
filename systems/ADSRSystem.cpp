#include "ADSRSystem.h"

INSTANCE_IMPL(ADSRSystem);

ADSRSystem::ADSRSystem() : ComponentSystemImpl<ADSRComponent>("adsr") { }

void ADSRSystem::DoUpdate(float dt) {
	for(std::map<Entity, ADSRComponent*>::iterator jt=components.begin(); jt!=components.end(); ++jt) {
		ADSRComponent* adsr = (*jt).second;

		if (!adsr->active && adsr->activationTime <= 0) {
			adsr->value = adsr->idleValue;
			adsr->activationTime = 0;
			continue;
		}

		if (adsr->active) {
			adsr->activationTime += dt;

			if (adsr->activationTime < adsr->attackTiming) {
				adsr->value = MathUtil::Lerp(
					adsr->idleValue,
					adsr->attackValue,
					adsr->activationTime / adsr->attackTiming);
			} else if (adsr->activationTime < (adsr->attackTiming + adsr->decayTiming)) {
				adsr->value = MathUtil::Lerp(
					adsr->attackValue,
					adsr->sustainValue,
					(adsr->activationTime - adsr->attackTiming) / adsr->decayTiming);
			} else {
				adsr->value = adsr->sustainValue;
			}
		} else {
			adsr->activationTime = MathUtil::Min(adsr->activationTime, adsr->releaseTiming);
			adsr->activationTime -= dt;

			adsr->value = MathUtil::Lerp(
					adsr->idleValue,
					adsr->sustainValue,
					adsr->activationTime / adsr->releaseTiming);
		}
	}
}
