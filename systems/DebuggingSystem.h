#pragma once


#include "System.h"


struct DebuggingComponent {
	DebuggingComponent() : showLinked(false), showHighligh(false) { }

	//for example autonomous agent... or transform
	bool showLinked;

	//himself
	bool showHighligh;
};

#define theDebuggingSystem DebuggingSystem::GetInstance()
#define DEBUGGING(entity) theDebuggingSystem.Get(entity)

UPDATABLE_SYSTEM(Debugging)

public:
	void switchActivate();
	void switchActivate(bool value);
#ifndef DEBUG
private:
#endif
	bool activate;
	std::vector<Entity> entities;

	void addEntities();

};

