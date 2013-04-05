#include "TaskAISystem.h"

INSTANCE_IMPL(TaskAISystem);

TaskAISystem::TaskAISystem() : ComponentSystemImpl<TaskAIComponent>("TaskAI") {
    /* nothing saved */
}

void TaskAISystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(TaskAI, entity, tc)

		if (!tc->taskToPerform.empty()) {
			// update first
			TaskAI* task = tc->taskToPerform[0]->update(entity, dt);

			if (task) {
				tc->taskToPerform.insert(tc->taskToPerform.begin(), task);
			} else if (tc->taskToPerform[0]->complete(entity)) {
				delete tc->taskToPerform[0];
				tc->taskToPerform.erase(tc->taskToPerform.begin());
			}
		}
    }
}

#if SAC_INGAME_EDITORS
void TaskAISystem::addEntityPropertiesToBar(Entity, TwBar*) {

}
#endif