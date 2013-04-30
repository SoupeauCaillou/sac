#include "ZSQDSystem.h"

#include <GL/glew.h>

INSTANCE_IMPL(ZSQDSystem);

ZSQDSystem::ZSQDSystem() : ComponentSystemImpl<ZSQDComponent>("ZSQD") {
    ZSQDComponent sc;
    //componentSerializer.add(new Property<glm::vec2>("linear_velocity", OFFSET(linearVelocity, tc), glm::vec2(0.001, 0)));
}

void ZSQDSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(ZSQD, a, sc)
    }
}

#if SAC_INGAME_EDITORS
void ZSQDSystem::addEntityPropertiesToBar(Entity entity, TwBar*) {
    ZSQDComponent* sc = Get(entity, false);
    if (!sc) return;

    //TwAddVarRW(bar, "velocity.X", TW_TYPE_FLOAT, &tc->linearVelocity.x, "group=ZSQD precision=2 step=0,01");
}
#endif
