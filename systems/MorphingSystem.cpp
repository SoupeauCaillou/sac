#include "MorphingSystem.h"

INSTANCE_IMPL(MorphingSystem);

MorphingSystem::MorphingSystem() : ComponentSystemImpl<MorphingComponent>("Morphing") { }

void MorphingSystem::DoUpdate(float dt) {
	for(std::map<Entity, MorphingComponent*>::iterator jt=components.begin(); jt!=components.end(); ++jt) {
		MorphingComponent* m = (*jt).second;

		if (!m->active || m->activationTime>m->timing) {
			m->active = false;
			m->activationTime = 0;
            for (unsigned int i=0; i<m->elements.size(); i++)
                m->elements[i]->ended = false;
			continue;
		}
		if (m->active) {
			m->activationTime += dt;
			m->value = MathUtil::Min(1.0f, m->activationTime/m->timing);
            for (unsigned int i=0; i<m->elements.size(); i++) {
                if (!m->elements[i]->ended) {
					m->elements[i]->lerp(MathUtil::Min(1.0f, m->value * m->elements[i]->coeff));
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
