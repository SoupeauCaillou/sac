#pragma once

#include "base/Entity.h"
#include <vector>

namespace ActivationMode {
    enum Enum { Instantaneous = 0, Fading };
}

namespace Action {
    enum Enum { Idle, Enabling, Disabling };
}

class EntityBatch {
    public:
    EntityBatch(float fadeinDuration = 0.5f, float fadeoutDuration = 0.5f);

    void setDurations(float fadein, float fadeout);
    void addEntity(Entity e);

    void enable(ActivationMode::Enum mode = ActivationMode::Instantaneous);
    void disable(ActivationMode::Enum mode = ActivationMode::Instantaneous);
    bool update(float dt);

    static void enableEntities(std::vector<Entity>& entities);
    static void disableEntities(std::vector<Entity>& entities);
    static void updateEntities(std::vector<Entity>& entities, float alpha);

    private:
    Action::Enum action;
    ActivationMode::Enum mode;
    std::vector<Entity> entities;
    float fadeinDuration, fadeoutDuration;
    float accum;
};
