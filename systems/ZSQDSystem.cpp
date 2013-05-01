#include "ZSQDSystem.h"

#include "base/Log.h"

#include <GL/glew.h>

INSTANCE_IMPL(ZSQDSystem);

ZSQDSystem::ZSQDSystem() : ComponentSystemImpl<ZSQDComponent>("ZSQD") {
    ZSQDComponent zc;
    componentSerializer.add(new Property<float>("friction_coeff", OFFSET(frictionCoeff, zc), 0.0001f));
    componentSerializer.add(new Property<float>("max_speed", OFFSET(maxSpeed, zc), 0.0001f));
    componentSerializer.add(new Property<float>("new_direction_coeff", OFFSET(newDirectionCoeff, zc), 0.0001f));
}

void ZSQDSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(ZSQD, a, zc)
        //decrease current speed
        zc->currentSpeed = glm::max(zc->currentSpeed - zc->frictionCoeff * zc->maxSpeed * dt, 0.f);
        //if someone added some new directions, update speed and direction
        if (zc->directions.size() > 0) {
            //DEBUG_LOGI(zc->directions.size() << " more directions");

            //calculate the average new direction from the whole new inputs
            glm::vec2 newDir(0, 0);
            for (auto it : zc->directions) {
                float norm = it.x * it.x + it.y * it.y;

                newDir.x += it.x / norm;
                newDir.y += it.y / norm;
            }
            //normalize the new direction
            if (newDir != glm::vec2(0.f, 0.f))
                newDir = glm::normalize(newDir);

            //DEBUG_LOGI("current dir: " << zc->currentDirection << " at speed " << zc->currentSpeed
            //    << "\nAnd new direction is: " << newDir << " at speed " << zc->maxSpeed);

            //calculate the weight of the new direction
            float weight = zc->newDirectionCoeff;
            if (newDir == glm::vec2(0.f, 0.f))
                weight = zc->newDirectionCoeff * 3;
            else if (zc->currentDirection == glm::vec2(0.f, 0.f))
                weight = 1.;

            //update the direction with ponderated directions
            zc->currentDirection = newDir * weight + zc->currentDirection * (1 - weight);

            //currentDirection norm must be <= 1
            if (glm::length(zc->currentDirection) > 1.f)
                zc->currentDirection = glm::normalize(zc->currentDirection);

            //reset speed to maxSpeed, because we had new inputs
            zc->currentSpeed = zc->maxSpeed;

            zc->directions.clear();
        }

        //if we are moving, update the position
        if (zc->currentSpeed > 0.f) {
            if (zc->currentDirection != glm::vec2(0.f, 0.f))
                TRANSFORM(a)->position += zc->currentDirection * zc->currentSpeed * dt;
        } else {
            zc->currentDirection = glm::vec2(0.f, 0.f);
        }
    }
}

#if SAC_INGAME_EDITORS
void ZSQDSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    ZSQDComponent* zc = Get(entity, false);
    if (!zc) return;

    TwAddVarRW(bar, "frictionCoeff", TW_TYPE_FLOAT, &zc->frictionCoeff, "group=ZSQD precision=2 step=0,1");
    TwAddVarRW(bar, "maxSpeed", TW_TYPE_FLOAT, &zc->maxSpeed, "group=ZSQD precision=2 step=0,1");
    TwAddVarRW(bar, "newDirectionCoeff", TW_TYPE_FLOAT, &zc->newDirectionCoeff, "group=ZSQD precision=2 step=0,1");

}
#endif
