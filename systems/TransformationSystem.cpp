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


#if SAC_INGAME_EDITORS
void TransformationSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    TransformationComponent* tc = Get(entity, false);
    if (!tc) return;
    TwAddVarRW(bar, "size.X", TW_TYPE_FLOAT, &tc->size.x, "group=Transformation precision=3 step=0,01");
    TwAddVarRW(bar, "size.Y", TW_TYPE_FLOAT, &tc->size.y, "group=Transformation precision=3 step=0,01");
    TwAddVarRW(bar, "position.X", TW_TYPE_FLOAT, &tc->position.x, "group=Transformation precision=3 step=0,01");
    TwAddVarRW(bar, "position.Y", TW_TYPE_FLOAT, &tc->position.y, "group=Transformation precision=3 step=0,01");
    TwAddVarRW(bar, "rotation", TW_TYPE_FLOAT, &tc->rotation, "group=Transformation step=0,01 precision=3");
    TwAddVarRW(bar, "Z", TW_TYPE_FLOAT, &tc->z, "group=Transformation precision=3 step=0,01");
}
#endif
