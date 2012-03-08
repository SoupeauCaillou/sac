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
			// phase A
			if (adsr->activationTime < adsr->attackTiming) {

					
				if (adsr->moding == Linear) {
				adsr->value = MathUtil::Lerp(
					adsr->idleValue,
					adsr->attackValue,
					adsr->activationTime / adsr->attackTiming);
				} else if (adsr->moding == Quadratic) {
					/*
					float tgt = 10;
					float a = -(10*adsr->attackTiming-adsr->attackValue+adsr->idleValue)/adsr->attackTiming;
					float b = tgt - 2*adsr->attackTiming*a;
					float c = adsr->idleValue;
					float x = adsr->activationTime;
					adsr->value = a*x*x+b*x+c;
					LOGI("%f", adsr->value);
					* */
					
					float z = adsr->activationTime / adsr->attackTiming;
					//float pol = adsr->idleValue + z * ( z * 8 - 5) * (adsr->attackValue-adsr->idleValue) / 3 ;
					float pol = adsr->idleValue + z * z * (adsr->attackValue - adsr->idleValue); 	  
					adsr->value =pol;
				}

			// phase D
			} else if (adsr->activationTime < (adsr->attackTiming + adsr->decayTiming)) {
				adsr->value = MathUtil::Lerp(
					adsr->attackValue,
					adsr->sustainValue,
					(adsr->activationTime - adsr->attackTiming) / adsr->decayTiming);
			// phase S
			} else {
				adsr->value = adsr->sustainValue;
			}
		//phase R
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
