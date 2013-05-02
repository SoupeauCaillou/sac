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
#include "util/DataFileParser.h"
#include "util/ComponentFactory.h"
#if SAC_DEBUG
#include "base/EntityManager.h"
#endif
#if SAC_INGAME_EDITORS
#include "AntTweakBar.h"
#endif

// #define SAC_USE_VECTOR_STORAGE 1

#define PERSISTENT_PROP
#define RUNTIME_PROP

#define INSTANCE_DECL(T) static T* _instance;
#define INSTANCE_IMPL(T) T* T::_instance = 0;

#if SAC_INGAME_EDITORS
#define UPDATABLE_SYSTEM(type) \
    class type##System : public ComponentSystemImpl<type##Component> {  \
        public: \
            static type##System* GetInstancePointer() { return _instance; } \
            static type##System& GetInstance() { return (*_instance); } \
            static void CreateInstance() {\
                if (_instance != NULL) {\
                    LOGW("Creating another instance of type##System")\
                }\
                _instance = new type##System();\
                LOGI(#type "System new instance created: "<<  _instance)\
            }\
            static void DestroyInstance() { \
                if (_instance) {\
                    delete _instance;\
                }\
                LOGI(#type << "System instance destroyed was: " <<  _instance)\
                _instance = NULL;\
            } \
            void DoUpdate(float dt); \
            void updateEntityComponent(float dt, Entity e, type##Component* t); \
        \
            type##System(); \
            void addEntityPropertiesToBar(Entity , TwBar* ); \
        private:    \
            static type##System* _instance;

#else
#define UPDATABLE_SYSTEM(type) \
    class type##System : public ComponentSystemImpl<type##Component> {  \
        public: \
            static type##System* GetInstancePointer() { return _instance; } \
			static type##System& GetInstance() { return (*_instance); } \
			static void CreateInstance() { \
			    if (_instance != NULL) { \
				    LOGW("Creating another instance of type##System")\
			    }\
			    _instance = new type##System();\
			    LOGI(#type "System new instance created: " <<  _instance)\
			} \
			static void DestroyInstance() {\
			    if (_instance)\
				    delete _instance;\
			    LOGI(#type "System instance destroyed, was:" << _instance)\
			    _instance = NULL;\
			} \
			void DoUpdate(float dt); \
			void updateEntityComponent(float dt, Entity e, type##Component* t); \
		private: \
			type##System();	\
			static type##System* _instance;

#endif

#if SAC_USE_VECTOR_STORAGE
    #define FOR_EACH_ENTITY(ent) \
        for(std::vector<EntityNextFree>::iterator it=entityWithComponent.begin(); it!=entityWithComponent.end();) { \
            Entity ent = *it;\
            ++it; \
            if (ent == 0) continue;

    #define FOR_EACH_ENTITY_COMPONENT(type, ent, comp) \
        for(unsigned ___i=0, ___s=components.size(); ___i<___s;) {\
            Entity ent = entityWithComponent[___i].entity;\
            if (ent == 0) { ++___i; continue; } \
            type##Component* comp = &components[___i]; \
            ++___i;
#else

    #define FOR_EACH_ENTITY(type, ent) \
         for(auto __it__ : the##type##System.getAllComponents()) { \
        LOGI("the#type##System");\
            Entity ent = __it__.first;

    #define FOR_EACH_ENTITY_COMPONENT(type, ent, comp) \
         for(auto __it__ : the##type##System.getAllComponents()) { \
            Entity ent = __it__.first;\
            type##Component* comp = __it__.second;

    #define FOR_EACH_COMPONENT(type, comp) \
         for(auto __it__ : the##type##System.getAllComponents()) { \
            type##Component* comp = __it__.second;

#endif

class ComponentSystem {
	public:
		ComponentSystem(const std::string& n) : name(n)
#if SAC_DEBUG
            , updateDuration(0)
#endif
         { bool inserted = registry.insert(std::make_pair(name, this)).second; LOGF_IF(!inserted, "System with name '" << name << "' already exists") }

        virtual ~ComponentSystem() { registry.erase(name); }

		virtual const std::string& getName() const { return name; }
		virtual void Add(Entity entity) = 0;
		virtual void Delete(Entity entity) = 0;
        virtual uint8_t* saveComponent(Entity entity, uint8_t* out = 0) = 0;
		virtual int serialize(Entity entity, uint8_t** out, void* ref = 0) = 0;
		virtual int deserialize(Entity entity, uint8_t* out, int size) = 0;
        virtual void applyEntityTemplate(Entity entity, const PropertyNameValueMap& propMap) = 0;

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
        virtual void addEntityPropertiesToBar(Entity , TwBar* ) {}
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
            DEBUG_LOGF_IF(components.find(entity) != components.end(), "Entity '" << theEntityManager.entityName(entity) << "' has the same component('" << getName() << "') twice!");
            T* comp = CreateComponent();

			components.insert(std::make_pair(entity, comp));
#endif
		}

#if SAC_DEBUG
		virtual void preDeletionCheck(Entity) { };
#endif

		void Delete(Entity entity) {
#if SAC_DEBUG
		    preDeletionCheck(entity);
#endif

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

        std::map<Entity, T*> getAllComponents() {
            return components;
        }

		T* Get(Entity entity, bool failIfNotfound = true) {
            if (entity != previous) {
#if SAC_USE_VECTOR_STORAGE
                std::map<Entity, unsigned>::iterator it = entityToIndice.find(entity);
                if (it == entityToIndice.end()) {
                    DEBUGLOGF_IF(failIfNotfound, "Entity '" << theEntityManager.entityName(entity) << "' has no component of type '" << getName() << "'")
                }
                previousComp = &components[it->second];
#else
    			ComponentIt it = components.find(entity);
    			if (it == components.end()) {
                    // crash here
                    if (failIfNotfound) {
                        DEBUG_LOGF("Entity '" << theEntityManager.entityName(entity)
                            << "' (" << entity << ") has no component of type '" << getName() << "'")
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

        uint8_t* saveComponent(Entity entity, uint8_t* out) {
            if (!out) {
                out = new uint8_t[sizeof(T)];
            }
            memcpy(out, Get(entity), sizeof(T));
            return out;
        }

		int serialize(Entity entity, uint8_t** out, void* ref) {
			T* component = Get(entity);
            return componentSerializer.serializeObject(out, component, ref);
		}

        void applyEntityTemplate(Entity entity, const PropertyNameValueMap& propMap) {
            T* comp = Get(entity);
            ComponentFactory::applyTemplate(entity, comp, propMap, componentSerializer.getProperties());
        }

		int deserialize(Entity entity, uint8_t* in, int size) {
			T* component = Get(entity, false);
            if (!component)
#if SAC_USE_VECTOR_STORAGE
                { Add(entity); component = Get(entity); }
#else
                component = new T();
#endif
            int s = componentSerializer.deserializeObject(in, size, component);
#if SAC_USE_VECTOR_STORAGE

#else
			components[entity] = component;
#endif
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
	protected:
        typedef typename std::map<Entity, T*> ComponentMap;
        typedef typename std::map<Entity, T*>::iterator ComponentIt;
        typedef typename std::map<Entity, T*>::const_iterator ComponentConstIt;
#endif
        Entity previous;
        T* previousComp;
};
