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



#include <UnitTest++.h>
#include "systems/AutonomousAgentSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/PhysicsSystem.h"

Entity groupEntity(glm::vec2 position, float rotationDegrees) {
    static int i = 1000;
    Entity e = i++;
    ADD_COMPONENT(e, Transformation);
    ADD_COMPONENT(e, Physics);
    TRANSFORM(e)->position = position;
    TRANSFORM(e)->rotation = glm::radians(rotationDegrees);
    return e;
}

TEST(GrooupAlignementBehavior)
{
    TransformationSystem::CreateInstance();
    PhysicsSystem::CreateInstance();

	Entity e = groupEntity(glm::vec2(0), 0);
	std::list<Entity> group;
	float maxSpeed = 1;

	glm::vec2 result = SteeringBehavior::groupAlign(e, group, maxSpeed);
	CHECK_CLOSE(0, result.x, 0.0001);
	CHECK_CLOSE(0, result.y, 0.0001);

    group.push_back(groupEntity(glm::vec2(1, 0), 90));
    result = SteeringBehavior::groupAlign(e, group, maxSpeed);
    CHECK_CLOSE(0, result.x, 0.0001);
    CHECK_CLOSE(1, result.y, 0.0001);

    group.push_back(groupEntity(glm::vec2(0, 1), 0));
    result = SteeringBehavior::groupAlign(e, group, maxSpeed);
    CHECK_CLOSE(1.f / glm::sqrt(2.f), result.x, 0.0001);
    CHECK_CLOSE(1.f / glm::sqrt(2.f), result.y, 0.0001);

    TransformationSystem::DestroyInstance();
    PhysicsSystem::DestroyInstance();
}
