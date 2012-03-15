#pragma once

#include <string>
#include <vector>
#include <map>
#include <stdint.h>
#include <iostream>
#include <cstring>
#include "base/Log.h"
#include "base/TimeUtil.h"

#include "DebugRenderingManager.h"

#define Entity unsigned long

#define INSTANCE_DECL(T) static T* _instance;
#define INSTANCE_IMPL(T) T* T::_instance = 0;

#define UPDATABLE_SYSTEM(type) \
	class type##System : public ComponentSystemImpl<type##Component> {	\
		public:	\
			static type##System& GetInstance() { return (*_instance); } \
			static void CreateInstance() { if (_instance != NULL) { LOGW("Creating another instance of type##System"); } _instance = new type##System(); } \
			static void DestroyInstance() { if (_instance) delete _instance; _instance = NULL; } \
			void DoUpdate(float dt); \
		\
		private:	\
			type##System();	\
			static type##System* _instance;

class ComponentSystem {
	public:
		ComponentSystem(const std::string& n) : name(n) { registry[name] = this; }

		virtual const std::string& getName() const { return name; }
		virtual void Add(Entity entity) = 0;
		virtual void Delete(Entity entity) = 0;
		virtual int serialize(Entity entity, uint8_t** out) = 0;
		virtual void deserialize(Entity entity, uint8_t* out, int size) = 0;

		void Update(float dt) {  float time = TimeUtil::getTime(); DoUpdate(dt); timeSpent = TimeUtil::getTime() - time; }
		float timeSpent;
			
		static ComponentSystem* Named(const std::string& n) {
			std::map<std::string, ComponentSystem*>::iterator it = registry.find(n);
			if (it == registry.end()) {
				std::cout << "System with name: '" << n << " does not exist" << std::endl;
				return 0;
			}
			return (*it).second;
		}
		
		static std::vector<std::string> registeredSystemNames();

	protected:
		virtual void DoUpdate(float dt) = 0;
		static std::map<std::string, ComponentSystem*> registry;
		std::string name;
};

template <typename T>
class ComponentSystemImpl: public ComponentSystem {
	public:
		ComponentSystemImpl(const std::string& t) : ComponentSystem(t) {
			Activate();
		}

		void Add(Entity entity) {
			components[entity] = new T();
		}

		void Delete(Entity entity) {
			typename std::map<Entity, T*>::iterator it = components.find(entity);
			if (it != components.end()) {
				components.erase(it);
				delete it->second;
			}
		}

		T* Get(Entity entity) {
			typename std::map<Entity, T*>::iterator it = components.find(entity);
			if (it == components.end()) {
				std::cout << "Entity " << entity << " has no component of type '" << name << "'" << std::endl;
				return 0;
			}
			return (*it).second;
		}

		std::vector<Entity> RetrieveAllEntityWithComponent() {
			std::vector<Entity> result;
			for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
				result.push_back((*it).first);
			}
			return result;
		}

		int serialize(Entity entity, uint8_t** out) {
			T* component = Get(entity);
			*out = new uint8_t[sizeof(T)];
			memcpy(*out, component, sizeof(T));
			return sizeof(T);
		}

		void deserialize(Entity entity, uint8_t* in, int size) {
			T* component = new T();
			if (size != sizeof(T)) {
				LOGW("error in size: %d != %d", size, sizeof(*component));
			}
			memcpy(component, in, size);
			components[entity] = component;
		}

		void Clear() { components.clear(); }

		void Activate() { active = true; }
		void Deactivate() { active = false; }
		bool IsActive() const { return active; }

	protected:
		bool active;

		std::map<Entity, T*> components;

		typedef typename std::map<Entity, T*> ComponentMap;
		typedef typename std::map<Entity, T*>::iterator ComponentIt;
		typedef typename std::map<Entity, T*>::const_iterator ComponentConstIt;

		float activationTime;
};
