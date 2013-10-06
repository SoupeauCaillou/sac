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



#include "FaderHelper.h"

#include "systems/AnchorSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"

FaderHelper::FaderHelper() : fadingEntity(0) {

}

void FaderHelper::init(Entity camera) {
	if (fadingEntity) {
		LOGF("FaderHelper already initialized");
	}

	duration = accum = 0;

	fadingEntity = theEntityManager.CreateEntity("fading");
	ADD_COMPONENT(fadingEntity, Rendering);
	RENDERING(fadingEntity)->color = Color(0,0,0);
	RENDERING(fadingEntity)->show = false;
	RENDERING(fadingEntity)->opaqueType = RenderingComponent::NON_OPAQUE;
	ADD_COMPONENT(fadingEntity, Anchor);
	ANCHOR(fadingEntity)->parent = camera;
	ANCHOR(fadingEntity)->z = 0.999 - TRANSFORM(camera)->z;
	ADD_COMPONENT(fadingEntity, Transformation);
	TRANSFORM(fadingEntity)->size = TRANSFORM(camera)->size;
}

void FaderHelper::registerFadingOutEntity(Entity e) {
	LOGW_IF(!RENDERING(e)->show, "Entity '" << theEntityManager.entityName(e) << "' registered as fading-out but isn't visible");
	fadingOut.push_back(e);
}

void FaderHelper::registerFadingInEntity(Entity e) {
	LOGW_IF(RENDERING(e)->show, "Entity '" << theEntityManager.entityName(e) << "' registered as fading-in but is already visible");
	fadingIn.push_back(e);
}

void FaderHelper::clearFadingEntities() {
	fadingOut.clear();
	fadingIn.clear();
}

static void updateFading(Fading::Enum type, Entity e, float progress) {
	switch (type) {
		case Fading::In:
			RENDERING(e)->color.a = 1 - progress;
			break;
		case Fading::Out:
			RENDERING(e)->color.a = progress;
			break;
		case Fading::OutIn:
			if (progress < 0.5) {
				updateFading(Fading::Out, e, 2 * progress);
			} else {
				updateFading(Fading::In, e, 2 * (progress - 0.5));
			}
			break;
	}

	TRANSFORM(e)->size = TRANSFORM(ANCHOR(e)->parent)->size;
}

void FaderHelper::start(Fading::Enum pType, float pDuration) {
	if (accum > 0 || duration > 0) {
		LOGF("Reentrant start for fading, expect weirdness");
	}
	LOGF_IF(pDuration <= 0, "Invalid duration: " << pDuration);
	accum = 0;
	duration = pDuration;
	type = pType;

	RENDERING(fadingEntity)->show = true;

	updateFading(type, fadingEntity, 0);
	LOGV(1, "Start fading");
}

bool FaderHelper::update(float dt) {
	if (duration <= 0)
		return true;
	const float oldProgress = accum / duration;
	accum = glm::min(duration, accum + dt);
	const float progress = accum / duration;
	updateFading(type, fadingEntity, progress);

	if (type == Fading::OutIn && oldProgress < 0.5 && progress >= 0.5) {
		// hide every fading-out entities
		for (auto e: fadingOut) {
			RENDERING(e)->show = false;
		}

		// show every fading-in entities
		for (auto e: fadingIn) {
			RENDERING(e)->show = true;
		}
	}

	if (progress >= 0.99) {
		accum = duration = 0;
		LOGV(1, "Fading done");
		if (type == Fading::In) {
			RENDERING(fadingEntity)->show = false;
		}
		return true;
	} else {
		return false;
	}
}
