/*
    This file is part of sac.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer

    RecursiveRunner is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    RecursiveRunner is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with RecursiveRunner.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "RenderTargetSystem.h"

INSTANCE_IMPL(RenderTargetSystem);

RenderTargetSystem::RenderTargetSystem() : ComponentSystemImpl<RenderTargetComponent>("RenderTarget") {

}

void RenderTargetSystem::DoUpdate(float dt) {

}

bool RenderTargetSystem::isDisabled(Entity e) {
    return !RENDER_TARGET(e)->enable;
}

bool RenderTargetSystem::sortAlongZ(Entity e, Entity f) {
    return RENDER_TARGET(e)->z < RENDER_TARGET(f)->z;
}
