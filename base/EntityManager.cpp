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

