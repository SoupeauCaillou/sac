#include "EntityManager.h"

EntityManager* EntityManager::instance = 0;

EntityManager::EntityManager() : nextEntity(1) {

}

EntityManager* EntityManager::Instance() {
	if (!instance) {
		instance = new EntityManager();
	}
	return instance;
}

Entity EntityManager::CreateEntity() {
	return nextEntity++;
}

void EntityManager::DeleteEntity(Entity e) {
	std::list<ComponentSystem*>& l = entityComponents[e];

	for (std::list<ComponentSystem*>::iterator it=l.begin(); it!=l.end(); ++it) {
		(*it)->Delete(e);
	}
	entityComponents.erase(e);
}

void EntityManager::AddComponent(Entity e, ComponentSystem* system) {
	system->Add(e);
	entityComponents[e].push_back(system);
}

