#include "Random.h"

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>

float Random::Float (float Min, float Max) {
	// return value is between [min; max]
	return glm::linearRand(Min, Max);
}

int Random::Int (int Min, int Max) {
	// return value is between [min; max]
	return ((((int)glm::linearRand((float) 0, (float) (Max-Min+1))) % (Max-Min+1))) + Min;
}