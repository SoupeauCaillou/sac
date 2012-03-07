#pragma once

#include "systems/System.h"
#define theEntityManager (*EntityManager::Instance())

#include <map>
#include <list>
#define ADD_COMPONENT(entity, type) theEntityManager.AddComponent((entity), &type##System::GetInstance())

class EntityManager {
	private:
		static EntityManager* instance;
		EntityManager();
	public:
		static EntityManager* Instance();

		enum EntityType {
			Volatile,
			Persistent
		};

	public:
		Entity CreateEntity(EntityType type = Volatile);
		void DeleteEntity(Entity e);
		void AddComponent(Entity e, ComponentSystem* system);
		void deleteAllEntities();
		std::vector<Entity> allEntities();

		int serialize(uint8_t** result);
		void deserialize(const uint8_t* in, int size);

	private:
		unsigned long nextEntity;
		std::map<Entity, std::list<ComponentSystem*> > entityComponents;
};

#ifdef ANDROID
void* mempcpy(void* dst, const void* src, size_t size);
#endif
