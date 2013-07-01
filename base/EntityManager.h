#pragma once

#include "Entity.h"
#define theEntityManager (*EntityManager::Instance())

#include <map>
#include <list>
#include <sstream>
#include <vector>
#if SAC_DEBUG
#include <string>
#endif
#define ADD_COMPONENT(entity, type) theEntityManager.AddComponent((entity), &type##System::GetInstance())

#include "systems/opengl/EntityTemplateLibrary.h"

class ComponentSystem;
class DataFileParser;
class AssetAPI;

#if SAC_EMSCRIPTEN || SAC_WINDOWS || SAC_DARWIN
void* mempcpy(void* dst, const void* src, size_t size);
#elif SAC_ANDROID
void* mempcpy(void* dst, const void* src, size_t size) throw();
#endif

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

		Entity CreateEntity(const std::string& name = "noname"
            , EntityType::Enum type = EntityType::Volatile,
            EntityTemplateRef tmpl = InvalidEntityTemplateRef);

    	void DeleteEntity(Entity e);
		void AddComponent(Entity e, ComponentSystem* system, bool failIfAlreadyHas = true);
		void deleteAllEntities();
		std::vector<Entity> allEntities();

		int serialize(uint8_t** result);
		void deserialize(const uint8_t* in, int size);

		Entity getEntityByName(const std::string& name) const;

#ifdef SAC_DEBUG
        const std::string& entityName(Entity e) const;
#else
        const inline std::string entityName(Entity e) const {
            static std::stringstream ss;
            ss << e;
            return ss.str();
        }
#endif
        int getNumberofEntity() {return entityComponents.size();}

	private:
		Entity nextEntity;
		std::map<Entity, std::list<ComponentSystem*> > entityComponents;
        std::map<Entity, std::string> entity2name;

    public:
        EntityTemplateLibrary entityTemplateLibrary;
};

void deleteEntityFunctor(Entity e);
