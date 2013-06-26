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
	ADD_COMPONENT(fadingEntity, Anchor);
	ANCHOR(fadingEntity)->parent = camera;
	ANCHOR(fadingEntity)->z = 0.999 - TRANSFORM(camera)->z;
	ADD_COMPONENT(fadingEntity, Transformation);
	TRANSFORM(fadingEntity)->size = TRANSFORM(camera)->size;
}

static void updateFading(Fading::Enum type, Entity e, float progress) {
	switch (type) {
		case Fading::In:
			RENDERING(e)->color.a = 1 - progress;
			break;
		case Fading::Out:
			RENDERING(e)->color.a = progress;
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
	accum = glm::min(duration, accum + dt);
	const float progress = accum / duration;
	updateFading(type, fadingEntity, progress);

	if (progress >= 0.99) {
		accum = duration = 0;
		RENDERING(fadingEntity)->show = false;
		LOGV(1, "Fading done");
		return true;
	} else {
		return false;
	}
}