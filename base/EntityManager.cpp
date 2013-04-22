#include "EntityManager.h"
#include <glm/glm.hpp>
#include "systems/System.h"
#include "util/DataFileParser.h"
#include <cstring>

#if SAC_ANDROID || SAC_EMSCRIPTEN || SAC_WINDOWS || SAC_DARWIN
void* mempcpy(void* dst, const void* src, size_t size)
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
	LOGV(1, "EntityTypeMask: " << EntityTypeMask << " (sizeof Entity: " << sizeof(Entity) << ", " <<  (sizeof(Entity) * 8 - 1))
}

EntityManager* EntityManager::Instance() {
	return instance;
}

void EntityManager::CreateInstance() {
	LOGW_IF(instance != 0, "Recreating EntityManager")
	instance = new EntityManager;
}

void EntityManager::DestroyInstance() {
	delete instance;
    instance = NULL;
}

Entity EntityManager::CreateEntity(const std::string& name, EntityType::Enum type) {
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
    entity2name[e] = name;

	return e;
}

void EntityManager::reloadEntity(Entity e, const DataFileParser& dfp) {
    const std::map<std::string, ComponentSystem*>& systems = ComponentSystem::registeredSystems();
    for (auto it = systems.begin(); it!=systems.end(); ++it) {
        if (dfp.hasSection(it->first)) {
            ComponentSystem* sys = it->second;
            sys->UpdateEntity(e, &dfp);
        }
    }
}

void EntityManager::reload(const std::string& name) {
    LOGI("Reload: '" << name << "'")
    auto it = file2entities.find(name);
    if (it == file2entities.end()) {
        LOGE("Unable to find entity: '" << name << "'")
        return;
    }
    DataFileParser dfp;
    FileBuffer fb = assetAPI->loadAsset(asset2File(name));
    if (!fb.size) {
        LOGE("Unable to load '" << asset2File(name) << "'")
        return;
    }
    if (!dfp.load(fb)) {
        LOGE("Unable to parse '" << asset2File(name) << "'")
        delete[] fb.data;
        return;
    }

    for (Entity e : it->second) {
        reloadEntity(e, dfp);
    }
    delete[] fb.data;
}

Entity EntityManager::CreateEntityFromFile(const std::string& name, EntityType::Enum type) {
    Entity e = CreateEntity(name, type);

    DataFileParser dfp;
    FileBuffer fb = assetAPI->loadAsset(asset2File(name));
    if (!fb.size) {
        LOGE("Unable to load '" << asset2File(name) << "'")
        return e;
    }
    if (!dfp.load(fb)) {
        LOGE("Unable to parse '" << asset2File(name) << "'")
        delete[] fb.data;
        return e;
    }

    LOGI( "Create entity: '" << name << "' from file")
    // browse system
    const std::map<std::string, ComponentSystem*>& systems = ComponentSystem::registeredSystems();
    for (auto it = systems.begin(); it!=systems.end(); ++it) {
        if (dfp.hasSection(it->first))
            AddComponent(e, it->second);
    }
    reloadEntity(e, dfp);
    delete[] fb.data;

#if SAC_LINUX && SAC_DESKTOP
    auto it = file2entities.find(name);
    if (it == file2entities.end()) {
        registerNewAsset(name);
    }
    file2entities[name].push_back(e);
#endif


    return e;
}


const std::string& EntityManager::entityName(Entity e) const {
    static const std::string u("unknown");
    auto it = entity2name.find(e);
    if (it != entity2name.end())
        return it->second;
    else
        return u;
}

void EntityManager::DeleteEntity(Entity e) {
	std::list<ComponentSystem*>& l = entityComponents[e];

	for (std::list<ComponentSystem*>::iterator it=l.begin(); it!=l.end(); ++it) {
		(*it)->Delete(e);
	}
	entityComponents.erase(e);

#if SAC_LINUX && SAC_DESKTOP
    auto it = entity2name.find(e);
    if (it != entity2name.end()) {
        auto jt = file2entities.find(it->second);
        if (jt != file2entities.end())
            jt->second.remove(e);
    }
#endif
    entity2name.erase(e);
}

void EntityManager::AddComponent(Entity e, ComponentSystem* system) {
	system->Add(e);
	entityComponents[e].push_back(system);
}

void EntityManager::deleteAllEntities() {
    std::vector<Entity> entities = allEntities();
    for (unsigned int i=0; i<entities.size(); i++) {
		DeleteEntity(entities[i]);
	}
	nextEntity = 1;
    assert (entityComponents.size() == 0);
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

	for (std::map<Entity, std::list<ComponentSystem*> >::iterator it=entityComponents.begin();
		it!=entityComponents.end();
		++it) {
		EntitySave e;
		e.e = (*it).first;

		if (!(e.e & EntityTypeMask))
			continue;

		totalLength += sizeof(Entity) + sizeof(int);

		for(std::list<ComponentSystem*>::iterator jt=(*it).second.begin();
			jt != (*it).second.end();
			++jt) {
			ComponentSave c;
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
		out = (uint8_t*)mempcpy(out, &saves[i].e, sizeof(Entity));
		int cCount = saves[i].components.size();
		out = (uint8_t*)mempcpy(out, &cCount, sizeof(int));
		for (int j=0; j<cCount; j++) {
			out = (uint8_t*)mempcpy(out, saves[i].components[j].name.c_str(), saves[i].components[j].name.length()+1);
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

		if (!(e & EntityTypeMask)) {
			LOGW("EntityManager deserializing a non-persistent entity")
		}

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
				LOGE("Failed to properly restore state: index=" << index << '/' << length << " i=" << i << "/" << cCount << ", entity=" << (e & ~EntityTypeMask))
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
        LOGV(1, " - restored entity '" << (e & ~EntityTypeMask) << "' with "  << l.size() << " components")
		nextEntity = glm::max(nextEntity, e + 1);
	}
}

void deleteEntityFunctor(Entity e) {
    theEntityManager.DeleteEntity(e);
}

std::string EntityManager::asset2File(const std::string& name) const {
    return "entities/" + name + ".entity";
}
