#pragma once

#include "systems/System.h"
#define theEntityManager (*EntityManager::Instance())

#include <map>
#include <list>
#define ADD_COMPONENT(entity, type) theEntityManager.AddComponent((entity), &type##System::GetInstance())

namespace EntityType {
    enum Enum {
        Volatile,
        Persistent
    };
}
class EntityManager {
	private:
		static EntityManager* instance;
		EntityManager();
	public:
		static EntityManager* Instance();
		static void CreateInstance();
		static void DestroyInstance();

	public:
		Entity CreateEntity(EntityType::Enum type = EntityType::Volatile);
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

#if defined(ANDROID) ||defined(EMSCRIPTEN)
void* mempcpy(void* dst, const void* src, size_t size);
#endif

void deleteEntityFunctor(Entity e);
