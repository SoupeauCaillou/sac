#pragma once

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

