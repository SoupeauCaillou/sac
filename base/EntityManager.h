#pragma once

#include "Entity.h"
#define theEntityManager (*EntityManager::Instance())

#include <map>
#include <list>
#include <vector>
#if SAC_DEBUG
#include <string>
#endif
#define ADD_COMPONENT(entity, type) theEntityManager.AddComponent((entity), &type##System::GetInstance())

#include "util/ResourceHotReload.h"

class ComponentSystem;
class DataFileParser;

namespace EntityType {
    enum Enum {
        Volatile,
        Persistent
    };
}
class EntityManager : public ResourceHotReload{
	private:
		static EntityManager* instance;
		EntityManager();
	public:
		static EntityManager* Instance();
		static void CreateInstance();
		static void DestroyInstance();

	public:

		Entity CreateEntity(const std::string& name = "noname"
            , EntityType::Enum type = EntityType::Volatile,
            const DataFileParser* dfp = 0);

    	void DeleteEntity(Entity e);
		void AddComponent(Entity e, ComponentSystem* system);
		void deleteAllEntities();
		std::vector<Entity> allEntities();

		int serialize(uint8_t** result);
		void deserialize(const uint8_t* in, int size);

#if SAC_DEBUG
        const std::string& entityName(Entity e) const;
#endif
        int getNumberofEntity() {return entityComponents.size();}

        // ResourceHotReload implem
        std::string asset2File(const std::string& name) const;
        void reload(const std::string& assetName);

	private:
		Entity nextEntity;
		std::map<Entity, std::list<ComponentSystem*> > entityComponents;
#if SAC_DEBUG
        std::map<std::string, Entity> name2entity;
#endif
};

void deleteEntityFunctor(Entity e);
