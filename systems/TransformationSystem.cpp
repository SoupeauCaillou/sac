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



#include "TransformationSystem.h"

INSTANCE_IMPL(TransformationSystem);

TransformationSystem::TransformationSystem() : ComponentSystemImpl<TransformationComponent>(HASH("Transformation", 0x4d33e992)) {
    TransformationComponent tc;
    componentSerializer.add(new Property<glm::vec2>(HASH("position", 0xffab91ef), OFFSET(position, tc), glm::vec2(0.001f, 0)));
    componentSerializer.add(new Property<glm::vec2>(HASH("size", 0x26d68039), OFFSET(size, tc), glm::vec2(0.001f, 0)));
    componentSerializer.add(new Property<float>(HASH("rotation", 0x18f19e94), OFFSET(rotation, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("z", 0x74729461), OFFSET(z, tc), 0.001f));
    componentSerializer.add(new Property<int>(HASH("shape", 0xb04443b9), OFFSET(shape, tc)));

    for (unsigned i=0; i<Shape::Count; i++) {
        shapes.push_back(Polygon::create((Shape::Enum)i));
    }

    LOGT("Move 'z' property to where it belongs: Rendering/Text (and remove from Anchor too)");
}

void TransformationSystem::DoUpdate(float) {
}

