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



#include "BackInTimeSystem.h"
#include "TransformationSystem.h"


INSTANCE_IMPL(BackInTimeSystem);

BackInTimeSystem::BackInTimeSystem() : ComponentSystemImpl<BackInTimeComponent>(HASH("BackInTime", 0x7c9eb7e5)) {
}

void BackInTimeSystem::DoUpdate(float) {
    FOR_EACH_ENTITY_COMPONENT(BackInTime, e, comp)
        const auto* tc = TRANSFORM(e);
        comp->position = tc->position;
        comp->size = tc->size;
        comp->rotation = tc->rotation;
    }
}
