#include "TransformationSystem.h"
#include <set>
#include <glm/gtx/rotate_vector.hpp>

INSTANCE_IMPL(TransformationSystem);

TransformationSystem::TransformationSystem() : ComponentSystemImpl<TransformationComponent>("Transformation") {
    TransformationComponent tc;
    componentSerializer.add(new Property<glm::vec2>("position", OFFSET(position, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<glm::vec2>("size", OFFSET(size, tc), glm::vec2(0.001, 0)));
    componentSerializer.add(new Property<float>("rotation", OFFSET(rotation, tc), 0.001));
    componentSerializer.add(new Property<float>("z", OFFSET(z, tc), 0.001));
}

void TransformationSystem::DoUpdate(float) {
}

