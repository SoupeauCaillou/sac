/*
    This file is part of sac.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer

    Heriswap is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Heriswap is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with sac.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CameraDepSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

INSTANCE_IMPL(CameraDepSystem);

CameraDepSystem::CameraDepSystem() : ComponentSystemImpl<CameraDepComponent>("CameraDep") {
    /* nothing saved */
}

void CameraDepSystem::DoUpdate(float dt) {
    std::vector<std::pair<Entity, bool> > toRemove;
    FOR_EACH_ENTITY_COMPONENT(CameraDep, a, cdc)
        const RenderingSystem::Camera& camera = theRenderingSystem.cameras[cdc->cameraIndex];
        TransformationComponent* tc = TRANSFORM(a);
        tc->position = camera.worldPosition + camera.worldSize * cdc->screenScalePosition;
        tc->size = camera.worldSize * cdc->screenScaleSize;
    }
}

