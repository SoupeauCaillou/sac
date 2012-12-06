#include <UnitTest++.h>
#include "systems/TaskAISystem.h"

TEST(OneTask)
{
	class TimerTask : public TaskAI {
		public:
			float timeLeft;

			TaskAI* update(Entity e  __attribute__((unused)), float dt) {
				timeLeft -= dt;
				return 0;
			}
			bool complete(Entity e  __attribute__((unused))) {
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
			TaskAI* update(Entity e __attribute__((unused)), float dt __attribute__((unused))) {
				*plop = true;
				return 0;
			}
			bool complete(Entity e __attribute__((unused))) {
				taskAComplete = true;
				return *plop;
			}
	};
	class TaskB : public TaskAI {
		public:
			bool subTaskComplete;

			TaskAI* update(Entity e __attribute__((unused)), float dt __attribute__((unused))) {
				if (!subTaskComplete) {
					return new TaskA(&subTaskComplete);
				} else {
					return 0;
				}
			}
			bool complete(Entity e __attribute__((unused))) {
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
