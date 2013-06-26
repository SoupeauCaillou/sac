#pragma once

#include "base/Entity.h"

namespace Fading {
	enum Enum {
		In,
		Out
	};
}

class FaderHelper {
	public:
		FaderHelper();

		void init(Entity camera);

		void start(Fading::Enum type, float duration);

		bool update(float dt);

	private:
		Entity fadingEntity;
		float duration;
		float accum;
		Fading::Enum type;
};