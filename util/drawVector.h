#pragma once

#include "base/Entity.h"
#include "base/Color.h"

#include <glm/glm.hpp>

Entity drawVector(const glm::vec2& position, const glm::vec2& size, Entity vector = 0, const Color & color = Color(1., 0., 0.));
