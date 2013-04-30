#include "ZSQDSystem.h"

#include "base/Log.h"

#include <GL/glew.h>

INSTANCE_IMPL(ZSQDSystem);

ZSQDSystem::ZSQDSystem() : ComponentSystemImpl<ZSQDComponent>("ZSQD") {
    ZSQDComponent zc;
    componentSerializer.add(new Property<float>("frictionCoeff", OFFSET(frictionCoeff, zc), 0.0001f));
    componentSerializer.add(new Property<float>("maxSpeed", OFFSET(maxSpeed, zc), 0.0001f));
}

void ZSQDSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(ZSQD, a, zc)
        //decrease current speed
        zc->currentSpeed = glm::max(zc->currentSpeed - zc->frictionCoeff * zc->maxSpeed, 0.f);
        //if someone added some new directions, update speed and direction
        if (zc->directions.size() > 0) {
            DEBUG_LOGI(zc->directions.size() << " more directions");

            //calculate the average new direction from the whole new inputs
            glm::vec2 newDir(0, 0);
            for (auto it : zc->directions) {
                float norm = it.x * it.x + it.y * it.y;

                newDir.x += it.x / norm;
                newDir.y += it.y / norm;
            }
            //normalize the extra direction
            if (newDir != glm::vec2(0.f, 0.f))
                newDir = glm::normalize(newDir);

            DEBUG_LOGI("current dir: " << zc->currentDirection.x << ", " << zc->currentDirection.y << " at speed " << zc->currentSpeed << " and new direction is: " << newDir.x << ", " << newDir.y << " at speed " << zc->maxSpeed);
            //current direction is the average from itself and the new direction. The coefficient of each is the speed associated

            zc->currentDirection = (zc->currentDirection * zc->currentSpeed + newDir * zc->maxSpeed) / (zc->currentSpeed + zc->maxSpeed);

            zc->currentDirection = (glm::length(zc->currentDirection) < 0.0001f) ?
                glm::vec2(0.f, 0.f)
                : glm::normalize(zc->currentDirection);

            zc->currentSpeed = zc->maxSpeed;

            zc->directions.clear();
        }

        if (zc->currentSpeed > 0.f && zc->currentDirection != glm::vec2(0.f, 0.f)) {
            DEBUG_LOGI("adding direction: " << zc->currentDirection.x << ", " << zc->currentDirection.y);
            TRANSFORM(a)->position += zc->currentDirection * zc->currentSpeed * dt;
        }
    }
}

#if SAC_INGAME_EDITORS
void ZSQDSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    ZSQDComponent* zc = Get(entity, false);
    if (!zc) return;

    TwAddVarRW(bar, "frictionCoeff", TW_TYPE_FLOAT, &zc->frictionCoeff, "group=ZSQD precision=2 step=0,1");
    TwAddVarRW(bar, "maxSpeed", TW_TYPE_FLOAT, &zc->maxSpeed, "group=ZSQD precision=2 step=0,1");
}
#endif
