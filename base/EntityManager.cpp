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

static Entity EntityTypeMask;

EntityManager* EntityManager::instance = 0;

EntityManager::EntityManager() : nextEntity(1) {
	EntityTypeMask = 1;
	EntityTypeMask <<= (sizeof(Entity) * 8 - 1);
	LOGV(1, "EntityTypeMask: " << EntityTypeMask << " (sizeof Entity: " << sizeof(Entity) << ", " <<  (sizeof(Entity) * 8 - 1));
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

Entity EntityManager::CreateEntity(const std::string& name, EntityType::Enum type, EntityTemplateRef tmpl) {
	Entity e = nextEntity++;

	switch (type) {
		case EntityType::Volatile:
			e &= ~((unsigned long)EntityTypeMask);
			break;
		case EntityType::Persistent:
			e |= EntityTypeMask;
			break;
	}

	// maybe hide the TypeBit from the rest of the world...
    LOGF_IF(entity2name.find(e) != entity2name.end(), "Newly created entity '" << e << "' is already in entity2name. Value='" << entity2name.find(e)->second << "'");
    entity2name[e] = name;

    if (tmpl != InvalidEntityTemplateRef) {
        // add component
        entityTemplateLibrary.applyEntityTemplate(e, tmpl);
    }
	return e;
}

Entity EntityManager::CreateEntityFromTemplate(const std::string& name, EntityType::Enum type) {
	const EntityTemplateRef tmpl = entityTemplateLibrary.load(name);
	LOGF_IF(tmpl == InvalidEntityTemplateRef, "Invalid entity template '" << name << "'");
	return CreateEntity(name, type, tmpl);
}


const std::string& EntityManager::entityName(Entity e) const {
    static const std::string u("unknown");
    auto it = entity2name.find(e);
    if (it != entity2name.end())
        return it->second;
    else
        return u;
}

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

Entity EntityManager::getEntityByName(const std::string& name) const {
	Entity byName = 0;
#if SAC_DEBUG
	bool found = false;
#endif
	for (const auto& p: entity2name) {
		if (p.second == name) {
			byName = p.first;
#if SAC_DEBUG
			if (found) {
				LOGW("Requesting entity by name, but multiple entities share the same name: '" << name << "'");
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
    entity2name.erase(e);
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
    LOGF_IF (entityComponents.size() != 0, "entityComponents not empty after deleting all entities");
}

std::vector<Entity> EntityManager::allEntities() {
	std::vector<Entity> out;
	for (std::map<Entity, std::list<ComponentSystem*> >::iterator it=entityComponents.begin();
		it!=entityComponents.end(); ++it) {
		out.push_back(it->first);
	}
	return out;
}

struct ComponentSave {
	std::string name;
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

	for (std::map<Entity, std::list<ComponentSystem*> >::iterator it=entityComponents.begin();
		it!=entityComponents.end();
		++it) {
		EntitySave e;
		e.e = (*it).first;

		if (!(e.e & EntityTypeMask))
			continue;

		totalLength += sizeof(Entity) + 2 * sizeof(int);
		totalLength += entity2name.find(e.e)->second.size();

		for(std::list<ComponentSystem*>::iterator jt=(*it).second.begin();
			jt != (*it).second.end();
			++jt) {
			ComponentSave c;
			// TODO: use a simple id/hash
			c.name = (*jt)->getName();
			c.contentSize = (*jt)->serialize(e.e, &c.content);
			e.components.push_back(c);

			totalLength += c.name.length() + 1;
			totalLength += c.contentSize + sizeof(int);
		}
		saves.push_back(e);
	}

	*result = new uint8_t[totalLength];
	uint8_t* out = *result;
	for(unsigned int i=0; i<saves.size(); i++) {
		// entity (id)
		out = (uint8_t*)mempcpy(out, &saves[i].e, sizeof(Entity));
		
		const std::string& name = entity2name.find(saves[i].e)->second;
		const int nameLength = name.size();
		// name length
		out = (uint8_t*) mempcpy(out, &nameLength, sizeof(int));
		// name
		out = (uint8_t*) mempcpy(out, name.c_str(), nameLength);

		// nb component
		const int cCount = saves[i].components.size();
		out = (uint8_t*)mempcpy(out, &cCount, sizeof(int));
		// components
		for (int j=0; j<cCount; j++) {
			out = (uint8_t*)mempcpy(out, saves[i].components[j].name.c_str(), saves[i].components[j].name.length()+1);
			out = (uint8_t*)mempcpy(out, &saves[i].components[j].contentSize, sizeof(int));
			out = (uint8_t*)mempcpy(out, saves[i].components[j].content, saves[i].components[j].contentSize);
		}
	}
	return totalLength;
}


void EntityManager::deserialize(const uint8_t* in, int length) {
	char tmp[512];
	int index = 0;
	while (index < length) {
		Entity e;
		memcpy(&e, &in[index], sizeof(e)); index += sizeof(e);

		if (!(e & EntityTypeMask)) {
			LOGW("EntityManager deserializing a non-persistent entity");
		}
		int nameLength = 0;
		memcpy(&nameLength, &in[index], sizeof(int)); index += sizeof(int);
		LOGW_IF(nameLength == 0 || (nameLength > 512), "Invalid entity name:" << nameLength);
		nameLength = glm::min(512, nameLength);
		memcpy(tmp, &in[index], nameLength); index += nameLength;
		tmp[nameLength] = '\0';
		entity2name[e] = tmp;

		int cCount = 0;
		memcpy(&cCount, &in[index], sizeof(int)); index += sizeof(int);

		std::list<ComponentSystem*> l;
		for (int i=0; i<cCount; i++) {
			char tmp[128];
			for(int j=0; j<128; j++) {
				tmp[j] = in[index++];
				if (tmp[j] == '\0')
					break;
			}
			ComponentSystem* system = ComponentSystem::Named(tmp);
			if (system == 0) {
				LOGE("Failed to properly restore state: index=" << index << '/' << length << " i=" << i << "/" << cCount << ", entity=" << (e & ~EntityTypeMask));
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
        LOGI( " - restored entity '" << (e & ~EntityTypeMask) << "' / '" << entityName(e) << "' with "  << l.size() << " components");
		nextEntity = glm::max(nextEntity, (e & ~((unsigned long)EntityTypeMask)) + 1);
	}
}

void deleteEntityFunctor(Entity e) {
    theEntityManager.DeleteEntity(e);
}
