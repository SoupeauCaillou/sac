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
    END_FOR_EACH()
}
