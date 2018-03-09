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

#include "util/SerializerProperty.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/compatibility.hpp>

/*
 *        /\
 *               /  \ D
 *       A      /    \    S
 *         /      \________
 *        /                        \ R
 *   /__________________\
 *      0
*/
INSTANCE_IMPL(ADSRSystem);

ADSRSystem::ADSRSystem() : ComponentSystemImpl<ADSRComponent>(HASH("ADSR", 0x971e8b1e)) {
    ADSRComponent a;

    componentSerializer.add(new Property<bool>(HASH("active", 0x9809cb8b), OFFSET(active, a)));
    componentSerializer.add(new Property<float>(HASH("value", 0xe408edcf), OFFSET(value, a), 0.001f));
    componentSerializer.add(new Property<float>(HASH("idle_value", 0x7a0a707b), OFFSET(idleValue, a), 0.001f));
    componentSerializer.add(new Property<float>(HASH("attack_value", 0x9700290), OFFSET(attackValue, a), 0.001f));
    componentSerializer.add(new Property<float>(HASH("attack_timing", 0x6ff1d700), OFFSET(attackTiming, a), 0.001f));
    componentSerializer.add(new Property<float>(HASH("decay_timing", 0xae168225), OFFSET(decayTiming, a), 0.001f));
    componentSerializer.add(new Property<float>(HASH("sustain_value", 0x9de8e3ec), OFFSET(sustainValue, a), 0.001f));
    componentSerializer.add(new Property<float>(HASH("release_timing", 0xf69a57f3), OFFSET(releaseTiming, a), 0.001f));
    componentSerializer.add(new Property<int>(HASH("attack_mode", 0xc72e47e1), OFFSET(attackMode, a)));
    componentSerializer.add(new Property<int>(HASH("decay_mode", 0x81f0adc6), OFFSET(decayMode, a)));
    componentSerializer.add(new Property<int>(HASH("release_mode", 0x13b67da2), OFFSET(releaseMode, a)));
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
