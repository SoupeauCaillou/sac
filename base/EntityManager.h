#pragma once

#include "Entity.h"
#define theEntityManager (*EntityManager::Instance())

#include <map>
#include <list>
#include <vector>
#ifdef DEBUG
#include <string>
#endif
#define ADD_COMPONENT(entity, type) theEntityManager.AddComponent((entity), &type##System::GetInstance())

class ComponentSystem;

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
		Entity CreateEntity(const std::string&
        #ifdef DEBUG
            name = "noname"
        #endif
            , EntityType::Enum type = EntityType::Volatile);

    	void DeleteEntity(Entity e);
		void AddComponent(Entity e, ComponentSystem* system);
		void deleteAllEntities();
		std::vector<Entity> allEntities();

		int serialize(uint8_t** result);
		void deserialize(const uint8_t* in, int size);

        #ifdef DEBUG
        const std::string& entityName(Entity e) const;
        #endif
	private:
		unsigned long nextEntity;
		std::map<Entity, std::list<ComponentSystem*> > entityComponents;
        #ifdef DEBUG
        std::map<std::string, Entity> name2entity;
        #endif
};

#if defined(ANDROID) ||defined(EMSCRIPTEN)
void* mempcpy(void* dst, const void* src, size_t size);
#endif

void deleteEntityFunctor(Entity e);
