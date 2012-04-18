#include "MorphingSystem.h"

INSTANCE_IMPL(MorphingSystem);

MorphingSystem::MorphingSystem() : ComponentSystemImpl<MorphingComponent>("Morphing") { }

void MorphingSystem::DoUpdate(float dt) {
	for(std::map<Entity, MorphingComponent*>::iterator jt=components.begin(); jt!=components.end(); ++jt) {
		MorphingComponent* m = (*jt).second;

		if ((!m->active && m->activationTime<=0) || m->activationTime>m->timing) {
			m->active = false;
			m->activationTime = 0;
			continue;
		}
		if (m->active) {
			m->activationTime += dt;
			m->value = MathUtil::Min(1.0f, m->activationTime/m->timing);
            for (int i=0; i<m->elements.size(); i++) {
                m->elements[i]->lerp(m->value);
                if (m->value == 1) {
                    delete m->elements[i];
                    m->elements.erase(m->elements.begin() + i);
                    i--;
                }
            }
		}
	}
}

