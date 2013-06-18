#include "MorphingSystem.h"

#include <glm/glm.hpp>

INSTANCE_IMPL(MorphingSystem);

MorphingSystem::MorphingSystem() : ComponentSystemImpl<MorphingComponent>("Morphing") {
    /* nothing saved */
    MorphingComponent mc;
    componentSerializer.add(new Property<bool>("active", OFFSET(active, mc)));
    componentSerializer.add(new Property<float>("value", OFFSET(value, mc), 0.001));
    componentSerializer.add(new Property<float>("activation_time", OFFSET(activationTime, mc), 0.001));
    componentSerializer.add(new Property<float>("timing", OFFSET(timing, mc), 0.001));
}

void MorphingSystem::DoUpdate(float dt) {
    FOR_EACH_COMPONENT(Morphing, m)
        if (!m->active || m->activationTime>m->timing) {
            m->active = false;
            m->activationTime = 0;
            for (unsigned int i=0; i<m->elements.size(); i++)
                m->elements[i]->ended = false;
            continue;
        }
        if (m->active) {
            m->activationTime += dt;
            m->value = glm::min(1.0f, m->activationTime/m->timing);
            for (unsigned int i=0; i<m->elements.size(); i++) {
                if (!m->elements[i]->ended) {
                    m->elements[i]->lerp(glm::min(1.0f, m->value * m->elements[i]->coeff));
                    if (m->value == 1) {
                        m->elements[i]->ended = true;
                    }
                }
            }
        }
    }
}

void MorphingSystem::reverse(MorphingComponent* mc) {
    for (unsigned int i=0; i<mc->elements.size(); i++) {
        mc->elements[i]->reverse();
    }
}

void MorphingSystem::clear(MorphingComponent* mc) {
    for (unsigned int i=0; i<mc->elements.size(); i++) {
        delete mc->elements[i];
    }
    mc->elements.clear();
}
