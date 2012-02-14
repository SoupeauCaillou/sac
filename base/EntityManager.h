#pragma once

#include "systems/System.h"
#define theEntityManager (*EntityManager::Instance())

#include <map>
#include <list>

class EntityManager {
	private:
		static EntityManager* instance;
		EntityManager();
	public:
		static EntityManager* Instance();

	public:
		Entity CreateEntity();
		void DeleteEntity(Entity e);
		void AddComponent(Entity e, ComponentSystem* system);

	private:
		unsigned long nextEntity;
		std::map<Entity, std::list<ComponentSystem*> > entityComponents; 
};
