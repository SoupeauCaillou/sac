/*
	This file is part of sac.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "../base/Vector2.h"
#include "../base/EntityManager.h"

class SteeringBehavior {
	public:
		struct WanderParams {
			float radius;
			float distance;
			float jitter;
			Vector2 target, debugTarget;
		};
	public:
		static Vector2 seek(Entity e, const Vector2& targetPos, float maxSpeed);
		
		static Vector2 flee(Entity e, const Vector2& targetPos, float maxSpeed);
		
		static Vector2 arrive(Entity e, const Vector2& targetPos, float maxSpeed, float deceleration);
     
        static Vector2 arrive(const Vector2& pos, const Vector2& linearVel,const Vector2& targetPos, float maxSpeed, float deceleration);
		
		static Vector2 wander(Entity e, WanderParams& params, float maxSpeed);
};