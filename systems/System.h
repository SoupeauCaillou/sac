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

#include <string>
#include <vector>
#include <map>
#include <stdint.h>
#include <iostream>
#include <cstring>
#include <climits>

#include "base/Entity.h"

#include "base/TimeUtil.h"
#include "base/Profiler.h"
#include "util/Serializer.h"
#include "util/ComponentFactory.h"
#include "base/EntityManager.h"

class LocalizeAPI;
#if SAC_INGAME_EDITORS
#include "AntTweakBar.h"
#endif

// #define SAC_USE_VECTOR_STORAGE 1

#define INSTANCE_IMPL(T) T* T::_instance = 0;

#define UPDATABLE_SYSTEM(type) \
    class type##System : public ComponentSystemImpl<type##Component> {  \
        public: \
            static type##System* GetInstancePointer() { return _instance; } \
            static type##System& GetInstance() { return (*_instance); } \
            static void CreateInstance() {\
                if (_instance != NULL) {\
                    LOGW("Creating another instance of type##System");\
                }\
                _instance = new type##System();\
                LOGV(1, #type "System new instance created: "<<  _instance);\
            }\
            static void DestroyInstance() { \
                if (_instance) {\
                    delete _instance;\
                }\
                LOGV(1, #type << "System instance destroyed was: " <<  _instance);\
                _instance = NULL;\
            } \
            void DoUpdate(float dt); \
            void updateEntityComponent(float dt, Entity e, type##Component* t); \
        \
            type##System(); \
        private:    \
            static type##System* _instance;

#if SAC_USE_VECTOR_STORAGE
    #define FOR_EACH_ENTITY(ent) \
        for(std::vector<EntityNextFree>::iterator __it##__LINE__##type##__=entityWithComponent.begin(); __it##__LINE__##type##__!=entityWithComponent.end();) { \
            Entity ent = *__it##__LINE__##type##__;\
            ++__it##__LINE__##type##__; \
            if (ent == 0) continue;

    #define FOR_EACH_ENTITY_COMPONENT(type, ent, comp) \
        for(unsigned ___i=0, ___s=components.size(); ___i<___s;) {\
            Entity ent = entityWithComponent[___i].entity;\
            if (ent == 0) { ++___i; continue; } \
            type##Component* comp = &components[___i]; \
            ++___i;
#else
    //'it' variable uses '__LINE__' macro def to ensure it will be unique within a loop
    //(eg FOR_EACH_ENTITY loop in another FOR_EACH_ENTITY loop)
    #define FOR_EACH_ENTITY(type, ent) \
        for(auto __it##__LINE__##type##__=the##type##System.getAllComponents().begin(); __it##__LINE__##type##__!=the##type##System.getAllComponents().end();) { \
            Entity ent = __it##__LINE__##type##__->first; ++__it##__LINE__##type##__;

    #define FOR_EACH_ENTITY_COMPONENT(type, ent, comp) \
         for(auto __it##__LINE__##type##__=the##type##System.getAllComponents().begin(); __it##__LINE__##type##__!=the##type##System.getAllComponents().end();) { \
            Entity ent = __it##__LINE__##type##__->first;\
            type##Component* comp = __it##__LINE__##type##__->second; \
            ++__it##__LINE__##type##__;

    #define FOR_EACH_COMPONENT(type, comp) \
         for(auto __it##__LINE__##type##__=the##type##System.getAllComponents().begin(); __it##__LINE__##type##__!=the##type##System.getAllComponents().end();) { \
            type##Component* comp = __it##__LINE__##type##__->second; \
            ++__it##__LINE__##type##__;

#endif
    //this macro is used to avoid IDE highlighting problems with brace missing...
    #define END_FOR_EACH() }

class ComponentSystem {
	public:
		ComponentSystem(const std::string& n) : name(n)
#if SAC_DEBUG
            , updateDuration(0)
#endif
         {
            bool inserted = registry.insert(std::make_pair(name, this)).second;
            LOGF_IF(!inserted, "System with name '" << name << "' already exists");
        }

        virtual ~ComponentSystem() { registry.erase(name); }

		virtual const std::string& getName() const { return name; }
		virtual void Add(Entity entity) = 0;
		virtual void Delete(Entity entity) = 0;
        virtual void suspendEntity(Entity entity) = 0;
        virtual void resumeEntity(Entity entity) = 0;
        virtual uint8_t* saveComponent(Entity entity, uint8_t* out = 0) = 0;
		virtual int serialize(Entity entity, uint8_t** out, void* ref = 0) = 0;
		virtual int deserialize(Entity entity, uint8_t* out, int size) = 0;
        virtual void applyEntityTemplate(Entity entity, const PropertyNameValueMap& propMap, LocalizeAPI* localizeAPI) = 0;
        virtual void* componentAsVoidPtr(Entity e) = 0;
        virtual unsigned entityCount() const = 0;
        virtual void forEachEntityDo(std::function<void(Entity)> func) = 0;

		void Update(float dt) {
            PROFILE("SystemUpdate", name, BeginEvent);
#if SAC_DEBUG
            float before = TimeUtil::GetTime();
#endif
            DoUpdate(dt);
#if SAC_DEBUG
            updateDuration = TimeUtil::GetTime() - before;
#endif
            PROFILE("SystemUpdate", name, EndEvent);
        }

		static ComponentSystem* Named(const std::string& n) {
			std::map<std::string, ComponentSystem*>::iterator it = registry.find(n);
			if (it == registry.end()) {
                LOGE("System with name: '" << n << "' does not exist");
				return 0;
			}
			return (*it).second;
		}

		static std::vector<std::string> registeredSystemNames();
        static const std::map<std::string, ComponentSystem*>& registeredSystems();

#if SAC_INGAME_EDITORS
        bool addEntityPropertiesToBar(Entity e, TwBar* bar);
#endif

	protected:
		virtual void DoUpdate(float dt) = 0;
		static std::map<std::string, ComponentSystem*> registry;
	protected:
		std::string name;
    Serializer componentSerializer;
    public:
        const Serializer& getSerializer() const {
            return componentSerializer;
        }
    public:
#if SAC_DEBUG
        float updateDuration;
#endif
};

template <typename T>
class ComponentSystemImpl: public ComponentSystem {
	public:
		ComponentSystemImpl(const std::string& t) : ComponentSystem(t) {
            previous = 0;
#if SAC_USE_VECTOR_STORAGE
            _freelist = UINT_MAX;
#endif
		}

		void Add(Entity entity) {
#if SAC_USE_VECTOR_STORAGE
            if (_freelist == UINT_MAX) {
                 entityWithComponent.push_back(EntityNextFree(entity));
                 components.push_back(T());
                 entityToIndice[entity] = entityWithComponent.size() - 1;
             } else {
                 unsigned idx = _freelist;
                 _freelist = entityWithComponent[idx].nextFree;
                 entityWithComponent[idx].entity = entity;
                 components[idx] = T();
                 entityToIndice[entity] = idx;
             }
            LOGF_IF(components.size() != entityWithComponent.size(), "Entity '" << theEntityManager.entityName(entity) << "' has the same component('" << getName() << "') twice!");
#else
            LOGF_IF(components.find(entity) != components.end(), "Entity '" << theEntityManager.entityName(entity) << "' has the same component('" << getName() << "') twice!");
            T* comp = CreateComponent();

			components.insert(std::make_pair(entity, comp));
#endif
		}

		virtual void Delete(Entity entity) {
#if SAC_USE_VECTOR_STORAGE
            std::map<Entity, unsigned>::iterator it = entityToIndice.find(entity);
            unsigned idx = it->second;
            entityWithComponent[idx].entity = 0;
            entityWithComponent[idx].nextFree = _freelist;
            entityToIndice.erase(it);
            _freelist = idx;
#else
			ComponentIt it = components.find(entity);
			if (it != components.end()) {
				delete it->second;
				components.erase(it);
			}
#endif
            if (previous == entity) {
                previous = 0;
                previousComp = 0;
            }
		}

        void suspendEntity(Entity entity) {
            ComponentIt it = components.find(entity);

            if (it != components.end()) {
                suspended[entity] = it->second;
                components.erase(it);
            }
            if (previous == entity) {
                previous = 0;
                previousComp = 0;
            }
        }

        void resumeEntity(Entity entity) {
            LOGF_IF(components.find(entity) != components.end(), "Resuming an entity '" << entity << "' which is already active");
            LOGF_IF(suspended.find(entity) == components.end(), "Resuming a not suspended entity " << entity);
            components[entity] = suspended.find(entity)->second;
        }

        const std::map<Entity, T*>& getAllComponents() const {
            return components;
        }

        unsigned entityCount() const {
            return components.size();
        }

        void* componentAsVoidPtr(Entity e) {
            return Get(e, false);
        }

#if SAC_DEBUG
        T* Get(Entity entity, bool failIfNotfound = true, const char* file = "\0", int line = 0) {
            theEntityManager.validateEntity(entity);
#else
        T* Get(Entity entity, bool failIfNotfound = true, const char* file = 0, int line = 0) {
#endif
            if (entity != previous) {
#if SAC_USE_VECTOR_STORAGE
                std::map<Entity, unsigned>::iterator it = entityToIndice.find(entity);
                if (it == entityToIndice.end()) {
                    LOGF_IF(failIfNotfound, "Entity '" << theEntityManager.entityName(entity) << "' has no component of type '" << getName() << "'")
                }
                previousComp = &components[it->second];
#else
                ComponentIt it = components.find(entity);
                if (it == components.end()) {
                    // crash here
                    if (failIfNotfound) {
                        LOGF("Entity '" << theEntityManager.entityName(entity)
                            << "' (" << entity << ") has no component of type '" << getName() << "' [@ " << file << ':' << line << ']');
                    }
                    return 0;
                }
                previousComp = (*it).second;
#endif
                previous = entity;
            }
            return previousComp;
        }

		std::vector<Entity> RetrieveAllEntityWithComponent() {
			std::vector<Entity> result;
#if SAC_USE_VECTOR_STORAGE
            for(std::map<Entity, unsigned>::iterator it=entityToIndice.begin(); it!=entityToIndice.end(); ++it) {
#else
            for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
#endif
				result.push_back((*it).first);
			}
			return result;
		}

        void forEachEntityDo(std::function<void(Entity)> func) {
            for (auto p : components) {
                func(p.first);
            }
        }

        void forEachECDo(std::function<void(Entity, T*)> func) {
            for (auto p : components) {
                func(p.first, p.second);
            }
        }

        uint8_t* saveComponent(Entity entity, uint8_t* out) {
            if (!out) {
                out = (uint8_t*)(new T);//new uint8_t[sizeof(T)];
            }
            T* t = (T*)out;
            *t = *Get(entity);
            return out;
        }

		int serialize(Entity entity, uint8_t** out, void* ref) {
			T* component = Get(entity);
            return componentSerializer.serializeObject(out, component, ref);
		}

        void applyEntityTemplate(Entity entity, const PropertyNameValueMap& propMap, LocalizeAPI* localizeAPI) {
            T* comp = Get(entity);
            ComponentFactory::applyTemplate(entity, comp, propMap, componentSerializer.getProperties(), localizeAPI);
        }

		int deserialize(Entity entity, uint8_t* in, int size) {
			T* component = Get(entity, false);
            if (!component) {
                theEntityManager.AddComponent(entity, this, true);
                component = Get(entity);
            }
            int s = componentSerializer.deserializeObject(in, size, component);
            return s;
		}

	protected:
        virtual T* CreateComponent() {
            return new T();
        }

#if SAC_USE_VECTOR_STORAGE
        struct EntityNextFree {
            EntityNextFree(Entity e) : entity(e), nextFree(0) {}
            Entity entity;
            unsigned nextFree;
        };
        std::vector<T> components;
        std::vector<EntityNextFree> entityWithComponent;
        std::map<Entity, unsigned> entityToIndice;
        unsigned _freelist;
#else


#if SAC_DEBUG
	public:
#endif
		std::map<Entity, T*> components;
        std::map<Entity, T*> suspended;
	protected:
        typedef typename std::map<Entity, T*> ComponentMap;
        typedef typename std::map<Entity, T*>::iterator ComponentIt;
        typedef typename std::map<Entity, T*>::const_iterator ComponentConstIt;
#endif
        Entity previous;
        T* previousComp;
};
