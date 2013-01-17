/*!
 * \file TaskAISystem.h
 * \brief 
 * \author Pierre-Eric Pelloux-Prayer
 * \author Gautier Pelloux-Prayer
 */
 #pragma once

#include "System.h"

/*! \class TaskAI
 *  \brief ? */
class TaskAI {
	public:
		TaskAI() { }
		virtual ~TaskAI() {}

        /*! \brief update and/or return a prerequesite task
         *  \param e
         *  \param dt
         *  \return ? */
		virtual TaskAI* update(Entity e, float dt) = 0;

        /*! \brief checks if task is finished
         *  \param e
         *  \return Returns true if task is finished */
        virtual bool complete(Entity e) = 0;
};

/*! \struct TaskAIComponent
 *  \brief ? */
struct TaskAIComponent {
	std::vector<TaskAI*> taskToPerform; //!< list of task to perform
};

#define theTaskAISystem TaskAISystem::GetInstance()
#define TASK_AI(e) theTaskAISystem.Get(e)

UPDATABLE_SYSTEM(TaskAI)

};

