#include "EntityBatch.h"

#include "systems/ButtonSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextSystem.h"

EntityBatch::EntityBatch(float in, float out) :
    action(Action::Idle), mode(ActivationMode::Instantaneous), fadeinDuration(in), fadeoutDuration(out) {

}

void EntityBatch::setDurations(float fadein, float fadeout) {
    fadeinDuration = fadein;
    fadeoutDuration = fadeout;
}

void EntityBatch::addEntity(Entity e) {
    entities.push_back(e);
}

void EntityBatch::enable(ActivationMode::Enum m) {
    action = Action::Enabling;
    mode = m;
    accum = 0;

    if (m == ActivationMode::Instantaneous) {
        EntityBatch::enableEntities(entities);
    }
}

void EntityBatch::disable(ActivationMode::Enum m) {
    action = Action::Disabling;
    mode = m;
    accum = 0;

    if (m == ActivationMode::Instantaneous) {
        EntityBatch::disableEntities(entities);
    }
}

bool EntityBatch::update(float dt) {
    switch (action) {
        case Action::Idle: return true;
        case Action::Enabling:
        case Action::Disabling:
            if (mode == ActivationMode::Fading) {
                if (entities.empty())
                    return true;

                // progressively show/hide entities
                float t = 0;
                if (action == Action::Enabling) {
                    t = accum / fadeinDuration;
                } else {
                    t = accum / fadeoutDuration;
                }
                accum += dt;

                if (t >= 1) {
                    if (action == Action::Enabling) {
                        EntityBatch::enableEntities(entities);
                    } else {
                        EntityBatch::disableEntities(entities);
                    }
                    action = Action::Idle;
                    return true;
                } else {
                    if (action == Action::Disabling) t = 1 - t;

                    EntityBatch::updateEntities(entities, t);
                    return false;
                }
            } else {
                action = Action::Idle;
                return true;
            }
        default:
            action = Action::Idle;
            return true;
    }
}

void EntityBatch::enableEntities(std::vector<Entity>& entities) {
    for (auto e: entities) {
        auto* rc = theRenderingSystem.Get(e, false);
        if (rc)
            rc->show = true;

        auto* tc = theTextSystem.Get(e, false);
        if (tc)
            tc->show = true;

        auto* bc = theButtonSystem.Get(e, false);
        if (bc)
            bc->enabled = true;
    }
}

void EntityBatch::disableEntities(std::vector<Entity>& entities) {
    // hide every fading-out entities
    for (auto e: entities) {
        auto* rc = theRenderingSystem.Get(e, false);
        if (rc)
            rc->show = false;

        auto* tc = theTextSystem.Get(e, false);
        if (tc)
            tc->show = false;

        auto* bc = theButtonSystem.Get(e, false);
        if (bc)
            bc->enabled = false;
    }
}

void EntityBatch::updateEntities(std::vector<Entity>& entities, float alpha) {
    for (auto e: entities) {
        auto* rc = theRenderingSystem.Get(e, false);
        if (rc) {
            rc->show = true;
            rc->color.a = alpha;
        }

        auto* tc = theTextSystem.Get(e, false);
        if (tc) {
            tc->show = true;
            tc->color.a = alpha;
        }

        auto* bc = theButtonSystem.Get(e, false);
        if (bc)
            bc->enabled = false;
    }
}
