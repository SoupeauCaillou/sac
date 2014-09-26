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



#include "EntityManager.h"
#include "systems/System.h"
#include <cstring>

#if SAC_ANDROID || SAC_WINDOWS || SAC_DARWIN
static void* mempcpy(void* dst, const void* src, size_t size)
#if SAC_ANDROID
throw()
#endif
{
    memcpy(dst, src, size);
    return (uint8_t*)dst + size;
}
#endif

EntityManager* EntityManager::instance = 0;

EntityManager::EntityManager() : nextEntity(1) {

}

EntityManager* EntityManager::Instance() {
    return instance;
}

void EntityManager::CreateInstance() {
    LOGW_IF(instance != 0, "Recreating EntityManager");
    instance = new EntityManager;
}

void EntityManager::DestroyInstance() {
    delete instance;
    instance = NULL;
}

#if SAC_INGAME_EDITORS
void EntityManager::renameEntity(Entity e, hash_t id) {
    entityHash[e] = id;
}
#endif

Entity EntityManager::CreateEntity(const hash_t id, EntityType::Enum type, EntityTemplateRef tmpl) {
    Entity e = 0;

    // Reuse id if possible
    if (recyclableEntities.empty()) {
        e = nextEntity++;
    } else {
        e = recyclableEntities.front();
        recyclableEntities.pop_front();
#if SAC_DEBUG
        entityDeletionTime.erase(e);
#endif
    }

    if (e >= entityHash.size())
        entityHash.resize(2 * (entityHash.size() + 1));
    entityHash[e] = id;

    if (tmpl != InvalidEntityTemplateRef) {
        // add component
        entityTemplateLibrary.applyEntityTemplate(e, tmpl);
    }

    // Tag
    switch (type) {
        case EntityType::Volatile:
            break;
        case EntityType::Persistent:
            permanentEntities.push_back(e);
            break;
    }
    return e;
}

Entity EntityManager::CreateEntityFromTemplate(const char* name, EntityType::Enum type) {
    const EntityTemplateRef tmpl = entityTemplateLibrary.load(name);
    LOGF_IF(tmpl == InvalidEntityTemplateRef, "Invalid entity template '" << name << "'");
    return CreateEntity(tmpl /*Murmur::Hash(name)*/, type, tmpl);
}


#if SAC_ENABLE_LOG || SAC_INGAME_EDITORS
const char* EntityManager::entityName(Entity e) const {
    static const char* u = "unknown";

    if (e >= entityHash.size()) {
        LOGE("Undefined (not initialiazed ?) entity '" << e << "' used");
        return u;
    }
    hash_t id = entityHash[e];
    if (id)
#if SAC_DEBUG
        return Murmur::lookup(id);
#else
        return u;
#endif
    else
        return u;
}
#endif

void EntityManager::SuspendEntity(Entity e) {
    auto i = entityComponents.find(e);
    LOGF_IF(i == entityComponents.end(), "SuspendEntity requested with invalid entity " <<
        e << " ('" << entityName(e) << "')");

    std::list<ComponentSystem*>& l = i->second;
    suspendedEntityComponents[e] = l;

    for (std::list<ComponentSystem*>::iterator it=l.begin(); it!=l.end(); ++it) {
        (*it)->suspendEntity(e);
    }
    entityComponents.erase(i);
}

void EntityManager::ResumeEntity(Entity e) {
    auto i = suspendedEntityComponents.find(e);
    LOGF_IF(i == suspendedEntityComponents.end(), "ResumeEntity requested with invalid entity " <<
        e << " ('" << entityName(e) << "')");

    std::list<ComponentSystem*>& l = i->second;
    entityComponents[e] = l;

    for (std::list<ComponentSystem*>::iterator it=l.begin(); it!=l.end(); ++it) {
        (*it)->resumeEntity(e);
    }
    suspendedEntityComponents.erase(i);
}

Entity EntityManager::getEntityByName(hash_t id) const {
    Entity byName = 0;
#if SAC_DEBUG
    bool found = false;
#endif

    const auto count = entityHash.size();
    for (unsigned i=0; i<count; i++) {
        if (entityHash[i] == id) {
            byName = i;
#if SAC_DEBUG
            if (found) {
                LOGW("Requesting entity by name, but multiple entities share the same id: '" << id << "' / name: " << Murmur::lookup(id));
            }
            found = true;
#else
            break;
#endif
        }
    }

    return byName;
}

void EntityManager::DeleteEntity(Entity e) {
    auto i = entityComponents.find(e);

#if SAC_DEBUG
    auto del = entityDeletionTime.find(e);
    LOGE_IF(del != entityDeletionTime.end(), "Entity already deleted at: " << del->second.first);
#endif
    LOGF_IF(i == entityComponents.end(), "DeleteEntity requested with invalid entity " <<
        e << "('" << entityName(e) << "') (did you already removed it?)");
#if SAC_DEBUG
    entityDeletionTime[e] = std::make_pair(TimeUtil::GetTime(), entityName(e));
#endif

    std::list<ComponentSystem*>& l = i->second;

    for (std::list<ComponentSystem*>::iterator it=l.begin(); it!=l.end(); ++it) {
        (*it)->Delete(e);
    }
    entityComponents.erase(e);


#if SAC_LINUX && SAC_DESKTOP
    entityTemplateLibrary.remove(e);
#endif
    {
        entityHash[e] = 0;
    }
    recyclableEntities.emplace_front(e);
    auto it = std::find(permanentEntities.begin(), permanentEntities.end(), e);
    if (it != permanentEntities.end())
        permanentEntities.erase(it);
}

#if SAC_DEBUG
void EntityManager::validateEntity(Entity e) const {
    auto it = entityDeletionTime.find(e);
    if (it != entityDeletionTime.end()) {
        LOGE("Entity '" << e << "' (" << it->second.second << " ) was deleted at: " << it->second.first);
    }
}
#endif

void EntityManager::AddComponent(Entity e, ComponentSystem* system, bool failIfAlreadyHas) {
    if (!failIfAlreadyHas) {
        const auto it = entityComponents[e];
        if (std::find(it.begin(), it.end(), system) != it.end())
            return;
    }
    system->Add(e);
    entityComponents[e].push_back(system);
}

void EntityManager::RemoveComponent(Entity e, ComponentSystem* system) {
    system->Delete(e);
    entityComponents[e].remove(system);
}

void EntityManager::deleteAllEntities() {
    std::vector<Entity> entities = allEntities();
    for (auto it=entities.rbegin(); it!=entities.rend(); ++it)
        DeleteEntity(*it);
    nextEntity = 1;
    recyclableEntities.clear();
    entityHash.clear();

    LOGF_IF (entityComponents.size() != 0, "entityComponents not empty after deleting all entities");
}

std::vector<Entity> EntityManager::allEntities() {
    std::vector<Entity> out;
    out.reserve(entityComponents.size());
    for (std::map<Entity, std::list<ComponentSystem*> >::iterator it=entityComponents.begin();
        it!=entityComponents.end(); ++it) {
        out.push_back(it->first);
    }
    return out;
}

struct ComponentSave {
    hash_t id;
    int contentSize;
    uint8_t* content;
};

struct EntitySave {
    Entity e;
    std::vector<ComponentSave> components;
};

int EntityManager::serialize(uint8_t** result) {
    std::vector<EntitySave> saves;
    int totalLength = 0;

    // Save entities. For each entity we write:
    //  * entity (id)
    //  * name length
    //  * name
    //  * component count
    //  * for each component of entity:
    //      * system name
    //      * component size
    //      * component

    for (auto _e: permanentEntities) {
        EntitySave e;
        e.e = _e;
        auto j = entityComponents.find(_e);
        LOGE_IF(j == entityComponents.end(), "Permanent entity found " << theEntityManager.entityName(e.e) << " without components");

        if (j == entityComponents.end())
            continue;

        totalLength += sizeof(Entity) + sizeof(hash_t) + sizeof(int);

        for(auto* sys: j->second) {
            ComponentSave c;
            c.id = sys->getId();
            c.contentSize = sys->serialize(e.e, &c.content);
            e.components.push_back(c);

            totalLength += sizeof(hash_t);
            totalLength += c.contentSize + sizeof(int);
        }
        saves.push_back(e);
    }

    *result = new uint8_t[totalLength];
    uint8_t* out = *result;
    for(unsigned int i=0; i<saves.size(); i++) {
        // entity (id)
        out = (uint8_t*)mempcpy(out, &saves[i].e, sizeof(Entity));
        // hash
        out = (uint8_t*) mempcpy(out, &entityHash[saves[i].e], sizeof(hash_t));

        // nb component
        const int cCount = saves[i].components.size();
        out = (uint8_t*)mempcpy(out, &cCount, sizeof(int));
        // components
        for (int j=0; j<cCount; j++) {
            out = (uint8_t*)mempcpy(out, &saves[i].components[j].id, sizeof(hash_t));
            out = (uint8_t*)mempcpy(out, &saves[i].components[j].contentSize, sizeof(int));
            out = (uint8_t*)mempcpy(out, saves[i].components[j].content, saves[i].components[j].contentSize);
        }
    }
    return totalLength;
}


void EntityManager::deserialize(const uint8_t* in, int length) {
    int index = 0;
    while (index < length) {
        Entity e;
        memcpy(&e, &in[index], sizeof(e)); index += sizeof(e);

        permanentEntities.push_back(e);

        hash_t id = 0;
        memcpy(&id, &in[index], sizeof(hash_t)); index += sizeof(hash_t);
        if (e >= entityHash.size())
            entityHash.resize(2 * (entityHash.size() + 1));
        entityHash[e] = id;

        int cCount = 0;
        memcpy(&cCount, &in[index], sizeof(int)); index += sizeof(int);

        std::list<ComponentSystem*> l;
        for (int i=0; i<cCount; i++) {
            hash_t systemId;
            memcpy(&systemId, &in[index], sizeof(hash_t));
            index += sizeof(hash_t);
            ComponentSystem* system = ComponentSystem::GetById(systemId);
            if (system == 0) {
                LOGE("Failed to properly restore state: index=" << index << '/' << length << " i=" << i << "/" << cCount << ", entity=" << e);
            }
            int size = 0;
            memcpy(&size, &in[index], sizeof(int)); index += sizeof(int);
            uint8_t* b = new uint8_t[size];
            memcpy(b, &in[index], size); index += size;
            system->deserialize(e, b, size);
            delete[] b;

            l.push_back(system);
        }
        entityComponents[e] = l;
        LOGI( " - restored entity '" << e << "' / '" << entityName(e) << "' with "  << l.size() << " components");
        nextEntity = glm::max(nextEntity, e + 1);
    }
}

void deleteEntityFunctor(Entity e) {
    theEntityManager.DeleteEntity(e);
}
