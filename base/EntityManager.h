#pragma once

#include "systems/System.h"
#define theEntityManager (*EntityManager::Instance())

class EntityManager {
	private:
		static EntityManager* instance;
		EntityManager();
	public:
		static EntityManager* Instance();

	public:
		Entity CreateEntity();

	private:
		unsigned long nextEntity;
		
};
