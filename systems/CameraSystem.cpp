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
#include "TransformationSystem.h"
#include <glm/gtx/rotate_vector.hpp>

INSTANCE_IMPL(CameraSystem);

CameraSystem::CameraSystem() : ComponentSystemImpl<CameraComponent>("Camera") {
    CameraComponent tc;
    componentSerializer.add(new Property<bool>("enable", OFFSET(enable, tc)));
    componentSerializer.add(new Property<int>("id", OFFSET(id, tc)));
    componentSerializer.add(new Property<int>("order", OFFSET(order, tc)));
    componentSerializer.add(new Property<Color>("clear_color", OFFSET(clearColor, tc)));
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
