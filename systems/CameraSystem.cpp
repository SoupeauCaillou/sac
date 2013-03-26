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
#include "CameraSystem.h"

INSTANCE_IMPL(CameraSystem);

CameraSystem::CameraSystem() : ComponentSystemImpl<CameraComponent>("Camera") {

}

void CameraSystem::DoUpdate(float) {
    // no need
}

bool CameraSystem::isDisabled(Entity e) {
    return !CAMERA(e)->enable;
}

bool CameraSystem::sort(Entity e, Entity f) {
    return CAMERA(e)->order < CAMERA(f)->order;
}

#ifdef SAC_INGAME_EDITORS
void CameraSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    CameraComponent* tc = Get(entity, false);
    if (!tc) return;
    TwAddVarRW(bar, "framebuffer", TW_TYPE_INT32, &tc->fb, "group=Camera");
    TwAddVarRW(bar, "clearColor", TW_TYPE_COLOR4F, &tc->clearColor, "group=Camera");
    TwAddVarRW(bar, "camEnabled", TW_TYPE_BOOLCPP, &tc->enable, "group=Camera");
    TwAddVarRW(bar, "order", TW_TYPE_INT32, &tc->order, "group=Camera");
    TwAddVarRW(bar, "cameraId", TW_TYPE_INT32, &tc->id, "group=Camera");
}
#endif
