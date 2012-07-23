/*
	This file is part of Heriswap.

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

#include "base/Log.h"

#include "System.h"

class TaskAI {
	public:
		TaskAI() { }
		virtual ~TaskAI() {}
		
		// update and/or return a prerequesite task
		virtual TaskAI* update(Entity e, float dt) = 0;
		// return true if task is finished
		virtual bool complete(Entity e) = 0;
};	

struct TaskAIComponent {
	std::vector<TaskAI*> taskToPerform;
};

#define theTaskAISystem TaskAISystem::GetInstance()
#define TASK_AI(e) theTaskAISystem.Get(e)

UPDATABLE_SYSTEM(TaskAI)

};

