/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
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
