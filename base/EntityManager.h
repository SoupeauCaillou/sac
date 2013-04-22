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
class AssetAPI;

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

        void setAssetAPI(AssetAPI* api) { assetAPI = api; }

	public:

		Entity CreateEntity(const std::string& name = "noname"
            , EntityType::Enum type = EntityType::Volatile);
        Entity CreateEntityFromFile(const std::string& filename = "noname"
            , EntityType::Enum type = EntityType::Volatile);

    	void DeleteEntity(Entity e);
		void AddComponent(Entity e, ComponentSystem* system);
		void deleteAllEntities();
		std::vector<Entity> allEntities();

		int serialize(uint8_t** result);
		void deserialize(const uint8_t* in, int size);

        const std::string& entityName(Entity e) const;
        int getNumberofEntity() {return entityComponents.size();}

        // ResourceHotReload implem
        std::string asset2File(const std::string& name) const;
        void reload(const std::string& assetName);

	private:
		Entity nextEntity;
		std::map<Entity, std::list<ComponentSystem*> > entityComponents;
        std::map<Entity, std::string> entity2name;
#if SAC_LINUX && SAC_DESKTOP
        std::map<std::string, std::list<Entity> > file2entities;
#endif
        AssetAPI* assetAPI;
        void reloadEntity(Entity e, const DataFileParser& dfp);
};

void deleteEntityFunctor(Entity e);
