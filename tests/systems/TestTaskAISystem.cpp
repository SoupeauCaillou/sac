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
#include "systems/TaskAISystem.h"

TEST(OneTask)
{
	class TimerTask : public TaskAI {
		public:
			float timeLeft;

			TaskAI* update(Entity, float dt) {
				timeLeft -= dt;
				return 0;
			}
			bool complete(Entity) {
				return timeLeft <= 0;
			}
	};

	TaskAISystem::CreateInstance();
	Entity e = 1;
	theTaskAISystem.Add(e);

	TimerTask* t = new TimerTask();
	t->timeLeft = 1.0;
	TASK_AI(e)->taskToPerform.push_back(t);

	for(int i=0; i<9; i++) {
		theTaskAISystem.Update(0.11);
		CHECK(!t->complete(0));
	}
	theTaskAISystem.Update(0.11);
	CHECK(TASK_AI(e)->taskToPerform.empty());

	TaskAISystem::DestroyInstance();
}

TEST(DerivedTask)
{
	static bool taskAComplete = false;
	class TaskA : public TaskAI {
		public:
			bool* plop;

			TaskA(bool* b) : plop(b) { *plop = false; }
			TaskAI* update(Entity, float) {
				*plop = true;
				return 0;
			}
			bool complete(Entity) {
				taskAComplete = true;
				return *plop;
			}
	};
	class TaskB : public TaskAI {
		public:
			bool subTaskComplete;

			TaskAI* update(Entity, float) {
				if (!subTaskComplete) {
					return new TaskA(&subTaskComplete);
				} else {
					return 0;
				}
			}
			bool complete(Entity) {
				return true;
			}
	};

	TaskAISystem::CreateInstance();
	Entity e = 1;
	theTaskAISystem.Add(e);

	CHECK(taskAComplete == false);
	TaskB* tB = new TaskB();
	TASK_AI(e)->taskToPerform.push_back(tB);

	theTaskAISystem.Update(0.1);
	CHECK(taskAComplete == false);
	CHECK(TASK_AI(e)->taskToPerform.size() == 2);

	theTaskAISystem.Update(0.1);
	CHECK(taskAComplete == true);
	CHECK(TASK_AI(e)->taskToPerform.size() == 1);

	TaskAISystem::DestroyInstance();

}
