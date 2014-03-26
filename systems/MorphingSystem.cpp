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



#include "MorphingSystem.h"

#include <glm/glm.hpp>

INSTANCE_IMPL(MorphingSystem);

MorphingSystem::MorphingSystem() : ComponentSystemImpl<MorphingComponent>("Morphing") {
    /* nothing saved */
    MorphingComponent mc;
    componentSerializer.add(new Property<bool>(Murmur::Hash("active"), OFFSET(active, mc)));
    componentSerializer.add(new Property<float>(Murmur::Hash("value"), OFFSET(value, mc), 0.001f));
    componentSerializer.add(new Property<float>(Murmur::Hash("activation_time"), OFFSET(activationTime, mc), 0.001f));
    componentSerializer.add(new Property<float>(Murmur::Hash("timing"), OFFSET(timing, mc), 0.001f));
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
    END_FOR_EACH()
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
