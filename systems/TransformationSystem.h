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



#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "System.h"


struct TransformationComponent {
	TransformationComponent(): position(0.0f), size(1.0f), rotation(0), z(0.5) {}

	glm::vec2 position, size;
	float rotation; //in radians
	float z;
};

#define theTransformationSystem TransformationSystem::GetInstance()
#if SAC_DEBUG
#define TRANSFORM(e) theTransformationSystem.Get(e,true,__FILE__,__LINE__)
#else
#define TRANSFORM(e) theTransformationSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Transformation)

public:
    template<typename T>
    static void appendVerticesTo(const TransformationComponent* tc, T& out);
};

template<typename T>
inline void TransformationSystem::appendVerticesTo(const TransformationComponent* tc, T& out) {
    const glm::vec2 hSize = tc->size * 0.5f;
    out.push_back(tc->position + glm::rotate(glm::vec2(hSize.x, hSize.y), tc->rotation));
    out.push_back(tc->position + glm::rotate(glm::vec2(-hSize.x, hSize.y), tc->rotation));
    out.push_back(tc->position + glm::rotate(glm::vec2(-hSize.x, -hSize.y), tc->rotation));
    out.push_back(tc->position + glm::rotate(glm::vec2(hSize.x, -hSize.y), tc->rotation));
}
