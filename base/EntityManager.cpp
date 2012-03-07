#include "EntityManager.h"
#include "MathUtil.h"
#include <cstring>

#ifdef ANDROID
void* mempcpy(void* dst, const void* src, size_t size) {
	memcpy(dst, src, size);
	return (uint8_t*)dst + size;
}
#endif

#define EntityTypeMask 0x8000000000000000

EntityManager* EntityManager::instance = 0;

EntityManager::EntityManager() : nextEntity(1) {

}

EntityManager* EntityManager::Instance() {
	return instance;
}

void EntityManager::CreateInstance() {
	if (instance)
		LOGW("Recreating EntityManager");
	instance = new EntityManager;
}

void EntityManager::DestroyInstance() {
	delete instance;
}

Entity EntityManager::CreateEntity(EntityType type) {
	Entity e = nextEntity++;

	switch (type) {
		case Volatile:
			e &= ~((unsigned long)EntityTypeMask);
			break;
		case Persistent:
			e |= EntityTypeMask;
			break;
	}
	// maybe hide the TypeBit from the rest of the world...
	return e;
}

void EntityManager::DeleteEntity(Entity e) {
	std::list<ComponentSystem*>& l = entityComponents[e];

	for (std::list<ComponentSystem*>::iterator it=l.begin(); it!=l.end(); ++it) {
		(*it)->Delete(e);
	}
	entityComponents.erase(e);
}

void EntityManager::AddComponent(Entity e, ComponentSystem* system) {
	system->Add(e);
	entityComponents[e].push_back(system);
}

void EntityManager::deleteAllEntities() {
	for (std::map<Entity, std::list<ComponentSystem*> >::iterator it=entityComponents.begin();
		it!=entityComponents.end(); ++it) {
		DeleteEntity(it->first);
	}
	nextEntity = 1;
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
	for(int i=0; i<saves.size(); i++) {
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
			LOGW("EntityManager deserializing a non-persistent entity");
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
			int size = 0;
			memcpy(&size, &in[index], sizeof(int)); index += sizeof(int);
			uint8_t* b = new uint8_t[size];
			memcpy(b, &in[index], size); index += size;
			system->deserialize(e, b, size);
			delete[] b;

			l.push_back(system);
		}
		entityComponents[e] = l;
		nextEntity = MathUtil::Max(nextEntity, e + 1);
	}
}
