#include "Random.h"

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

void Random::Init () {
	//not a proper init but we don't mind (game!)
	srand(time(0));
}

float Random::Float (float min, float max) {
	return glm::linearRand(min, max);
}

int Random::Int (int min, int max) {
	return ((((int)glm::linearRand((float) 0, (float) (max-min+1))) % (max-min+1))) + min;
}
