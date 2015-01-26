#pragma once

#include "StateMachine.h"
#include "util/EntityBatch.h"

namespace SceneEntityMode {
    enum Enum {
        DoNothing,
        InstantaneousOnPreEnter,
        Fading,
        InstantaneousOnEnter,
        InstantaneousOnPreExit,
        InstantaneousOnExit,
    };
}

template <typename T> class SceneState : public StateHandler<T> {
    public:
    SceneState(const char* pName,
               SceneEntityMode::Enum _enterMode,
               SceneEntityMode::Enum _exitMode)
        : StateHandler<T>(pName), enterMode(_enterMode), exitMode(_exitMode) {}

    virtual void setup(AssetAPI* asset) {
        initStateEntities(asset, this->name, entities);
        for (auto e : entities) { this->batch.addEntity(e.second); }
    }

    virtual void onPreEnter(T) {
        switch (enterMode) {
        case SceneEntityMode::InstantaneousOnPreEnter:
            batch.enable(ActivationMode::Instantaneous);
            break;
        case SceneEntityMode::Fading:
            batch.enable(ActivationMode::Fading);
            break;
        default: break;
        }
    }

    virtual bool updatePreEnter(T, float dt) { return batch.update(dt); }

    virtual void onEnter(T) {
        switch (enterMode) {
        case SceneEntityMode::InstantaneousOnEnter:
            batch.enable(ActivationMode::Instantaneous);
            break;
        default: break;
        }
    }

    virtual void onPreExit(T) {
        switch (exitMode) {
        case SceneEntityMode::InstantaneousOnPreExit:
            batch.disable(ActivationMode::Instantaneous);
            break;
        case SceneEntityMode::Fading:
            batch.disable(ActivationMode::Fading);
            break;
        default: break;
        }
    }

    virtual bool updatePreExit(T, float dt) { return batch.update(dt); }

    virtual void onExit(T) {
        switch (exitMode) {
        case SceneEntityMode::InstantaneousOnExit:
            batch.disable(ActivationMode::Instantaneous);
            break;
        default: break;
        }
    }

    Entity e(hash_t h) { return entities[h]; }

    std::map<hash_t, Entity> entities;
    EntityBatch batch;
    SceneEntityMode::Enum enterMode, exitMode;
};
