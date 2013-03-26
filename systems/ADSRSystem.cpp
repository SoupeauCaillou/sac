#include "ADSRSystem.h"

#include "base/MathUtil.h"

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

#ifdef INGAME_EDITORS
void ADSRSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    ADSRComponent* tc = Get(entity, false);
    if (!tc) return;
    TwAddVarRW(bar, "active", TW_TYPE_BOOLCPP, &tc->active, "group=ADSR");
    TwAddVarRO(bar, "value", TW_TYPE_FLOAT, &tc->value, "group=ADSR");
    TwAddVarRO(bar, "activationTime", TW_TYPE_FLOAT, &tc->activationTime, "group=ADSR");
    TwAddVarRW(bar, "activationTime", TW_TYPE_FLOAT, &tc->activationTime, "group=ADSR");
    
    TwAddVarRW(bar, "idleValue", TW_TYPE_FLOAT, &tc->idleValue, "group=ADSR"); 
    TwAddVarRW(bar, "attackValue", TW_TYPE_FLOAT, &tc->attackValue, "group=ADSR"); 
    TwAddVarRW(bar, "attackTiming", TW_TYPE_FLOAT, &tc->attackTiming, "group=ADSR"); 
    TwAddVarRW(bar, "sustainValue", TW_TYPE_FLOAT, &tc->sustainValue, "group=ADSR"); 
    TwAddVarRW(bar, "decayTiming", TW_TYPE_FLOAT, &tc->decayTiming, "group=ADSR"); 
    TwAddVarRW(bar, "releaseTiming", TW_TYPE_FLOAT, &tc->releaseTiming, "group=ADSR");

    TwEnumVal adsrModes[] = { {Linear, "Linear"}, {Quadratic, "Quadratic"} };
    TwType adsrType = TwDefineEnum("ADSRMode", adsrModes, 2);
    TwAddVarRW(bar, "attackMode", adsrType, &tc->attackMode, "group=ADSR");
    TwAddVarRW(bar, "decayMode", adsrType, &tc->decayMode, "group=ADSR");
    TwAddVarRW(bar, "releaseMode", adsrType, &tc->releaseMode, "group=ADSR");
}
#endif
