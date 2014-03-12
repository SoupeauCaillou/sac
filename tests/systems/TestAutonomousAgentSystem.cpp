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

Entity obstacle(glm::vec2 position) {
    static int i = 1000;
    Entity e = i++;
    ADD_COMPONENT(e, Transformation);
    TRANSFORM(e)->position = position;
    return e;
}

TEST(AvoidSteeringBehavior)
{
 //    TransformationSystem::CreateInstance();

	// Entity e = 1;
	// ADD_COMPONENT(e, Transformation);
	// glm::vec2 velocity(1, 0);
	// std::list<Entity> obstacles;
	// float maxSpeed = 1;

	// glm::vec2 result = SteeringBehavior::avoid(e, velocity, obstacles, maxSpeed);
	// CHECK_CLOSE(0, result.x, 0.0001);
	// CHECK_CLOSE(0, result.y, 0.0001);

 //    obstacles.push_back(obstacle(glm::vec2(0.5, 0)));
 //    result = SteeringBehavior::avoid(e, velocity, obstacles, maxSpeed);
 //    CHECK_CLOSE(-.5, result.x, 0.0001);
 //    CHECK_CLOSE(0, result.y, 0.0001);

 //    obstacles.push_back(obstacle(glm::vec2(.125, .125)));
 //    result = SteeringBehavior::avoid(e, velocity, obstacles, maxSpeed);
 //    CHECK_CLOSE(-.125, result.x, 0.0001);
 //    CHECK_CLOSE(-.125, result.y, 0.0001);

 //    TransformationSystem::DestroyInstance();
}
