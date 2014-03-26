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

TransformationSystem::TransformationSystem() : ComponentSystemImpl<TransformationComponent>("Transformation") {
    TransformationComponent tc;
    componentSerializer.add(new Property<glm::vec2>(Murmur::Hash("position"), OFFSET(position, tc), glm::vec2(0.001f, 0)));
    componentSerializer.add(new Property<glm::vec2>(Murmur::Hash("size"), OFFSET(size, tc), glm::vec2(0.001f, 0)));
    componentSerializer.add(new Property<float>(Murmur::Hash("rotation"), OFFSET(rotation, tc), 0.001f));
    componentSerializer.add(new Property<float>(Murmur::Hash("z"), OFFSET(z, tc), 0.001f));
    componentSerializer.add(new Property<int>(Murmur::Hash("shape"), OFFSET(shape, tc)));

    for (unsigned i=0; i<Shape::Count; i++) {
        shapes.push_back(Polygon::create((Shape::Enum)i));
    }
}

void TransformationSystem::DoUpdate(float) {
}

