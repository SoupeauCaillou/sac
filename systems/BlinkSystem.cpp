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

#if !DISABLE_BLINK_SYSTEM

#include "BlinkSystem.h"
#include "RenderingSystem.h"

INSTANCE_IMPL(BlinkSystem);

BlinkSystem::BlinkSystem() : ComponentSystemImpl<BlinkComponent>(HASH("Blink", 0x0)) {
    BlinkComponent tc;
    componentSerializer.add(new Property<bool>(HASH("enabled", 0x1d6995b7), OFFSET(enabled, tc)));
    componentSerializer.add(new Property<float>(HASH("visible_duration", 0x40000832), OFFSET(visibleDuration, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("hidden_duration", 0x5e159338), OFFSET(hiddenDuration, tc), 0.001f));
}

void BlinkSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Blink, entity, bc)
        if (!bc->enabled) continue;

        bc->accum += dt;

        RENDERING(entity)->show = (bc->accum < bc->visibleDuration);

        float total = bc->visibleDuration + bc->hiddenDuration;
        LOGF_IF(total <= 0, "Invalid params: " <<bc->visibleDuration << "/" << bc->hiddenDuration);
        while (bc->accum > total) {
            bc->accum -= total;
        }
    END_FOR_EACH()
}
#endif
