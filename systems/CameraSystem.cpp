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



#include "CameraSystem.h"
#include "TransformationSystem.h"
#include <glm/gtx/rotate_vector.hpp>

INSTANCE_IMPL(CameraSystem);

CameraSystem::CameraSystem() : ComponentSystemImpl<CameraComponent>("Camera") {
    CameraComponent tc;
    componentSerializer.add(new Property<bool>(HASH("enable", 0x0), OFFSET(enable, tc)));
    componentSerializer.add(new Property<bool>(HASH("clear", 0x0), OFFSET(clear, tc)));
    componentSerializer.add(new Property<int>(HASH("id", 0x0), OFFSET(id, tc)));
    componentSerializer.add(new Property<int>(HASH("order", 0x0), OFFSET(order, tc)));
    componentSerializer.add(new Property<Color>(HASH("clear_color", 0x0), OFFSET(clearColor, tc)));
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

glm::vec2 CameraSystem::WorldToScreen(const TransformationComponent* tc, const glm::vec2& pos) {
    glm::vec2 p = glm::rotate(pos - tc->position, - tc->rotation);
    p /= tc->size;
    return p;
}

glm::vec2 CameraSystem::ScreenToWorld(const TransformationComponent* tc, const glm::vec2& pos) {
    return tc->position + glm::rotate(pos * tc->size, tc->rotation);
}
