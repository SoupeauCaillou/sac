/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



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

		Entity CreateEntityFromTemplate(const std::string& name = "noname"
            , EntityType::Enum type = EntityType::Volatile);

    	void DeleteEntity(Entity e);
		void AddComponent(Entity e, ComponentSystem* system, bool failIfAlreadyHas = true);
		void deleteAllEntities();
		std::vector<Entity> allEntities();

		int serialize(uint8_t** result);
		void deserialize(const uint8_t* in, int size);

		Entity getEntityByName(const std::string& name) const;

        const std::string& entityName(Entity e) const;

        int getNumberofEntity() {return entityComponents.size();}

	private:
		Entity nextEntity;
		std::map<Entity, std::list<ComponentSystem*> > entityComponents;
        std::map<Entity, std::string> entity2name;

    public:
        EntityTemplateLibrary entityTemplateLibrary;
};

void deleteEntityFunctor(Entity e);
