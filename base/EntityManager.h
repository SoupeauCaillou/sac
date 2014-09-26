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
#include <forward_list>
#include <vector>
#if SAC_DEBUG
#include <string>
#endif
#define ADD_COMPONENT(entity, type) theEntityManager.AddComponent((entity), &type##System::GetInstance())

#include "systems/opengl/EntityTemplateLibrary.h"

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

        Entity CreateEntity(const hash_t id
            , EntityType::Enum type = EntityType::Volatile,
            EntityTemplateRef tmpl = InvalidEntityTemplateRef);

        Entity CreateEntityFromTemplate(const char* name = "noname"
            , EntityType::Enum type = EntityType::Volatile);

        void DeleteEntity(Entity e);
        void AddComponent(Entity e, ComponentSystem* system, bool failIfAlreadyHas = true);
        void RemoveComponent(Entity e, ComponentSystem* system);
        void deleteAllEntities();
        std::vector<Entity> allEntities();

        void SuspendEntity(Entity e);
        void ResumeEntity(Entity e);

        int serialize(uint8_t** result);
        void deserialize(const uint8_t* in, int size);

        Entity getEntityByName(hash_t id) const;

#if SAC_ENABLE_LOG || SAC_INGAME_EDITORS
        const char* entityName(Entity e) const;
#endif
#if SAC_INGAME_EDITORS
        void renameEntity(Entity e, hash_t id);
#endif

        int getNumberofEntity() {return entityComponents.size();}

#if SAC_DEBUG
        void validateEntity(Entity e) const;
#endif
    private:
        Entity nextEntity;
        std::forward_list<Entity> recyclableEntities;

        std::list<Entity> permanentEntities;
        std::vector<hash_t> entityHash;
        std::map<Entity, std::list<ComponentSystem*> > entityComponents;
        std::map<Entity, std::list<ComponentSystem*> > suspendedEntityComponents;

#if SAC_DEBUG
        std::map<Entity, std::pair<float, std::string> > entityDeletionTime;
#endif
    public:
        EntityTemplateLibrary entityTemplateLibrary;
};

void deleteEntityFunctor(Entity e);
