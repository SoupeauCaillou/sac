#pragma once

#include <string>
#include <vector>
#include <map>
#include <stdint.h>
#include <iostream>
#include <cstring>
#include <climits>
#include <cassert>

#include "base/Entity.h"
#include <glog/logging.h>

#include "base/TimeUtil.h"
#include "base/Profiler.h"
#include "util/Serializer.h"
#ifdef DEBUG
#include "base/EntityManager.h"
#endif
#ifdef INGAME_EDITORS
#include "libs/AntTweakBar/include/AntTweakBar.h"
#endif

// #define USE_VECTOR_STORAGE 1

#define PERSISTENT_PROP
#define RUNTIME_PROP

#define INSTANCE_DECL(T) static T* _instance;
#define INSTANCE_IMPL(T) T* T::_instance = 0;

#ifdef INGAME_EDITORS
#define UPDATABLE_SYSTEM(type) \
    class type##System : public ComponentSystemImpl<type##Component> {  \
        public: \
            static type##System* GetInstancePointer() { return _instance; } \
            static type##System& GetInstance() { return (*_instance); } \
            static void CreateInstance() { if (_instance != NULL) { LOG(WARNING) << "Creating another instance of type##System"; } _instance = new type##System(); LOG(INFO) << #type "System new instance created: "<<  _instance;} \
            static void DestroyInstance() { \
                if (_instance) {\
                    delete _instance;\
                }\
                LOG(INFO) << #type << "System instance destroyed was: " <<  _instance;\
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
				    LOG(WARNING) << "Creating another instance of type##System";\
			    }\
			    _instance = new type##System();\
			    LOG(INFO) << #type "System new instance created: " <<  _instance;\
			} \
			static void DestroyInstance() {\
			    if (_instance)\
				    delete _instance;\
			    LOG(INFO) << #type "System instance destroyed, was:" << _instance;\
			    _instance = NULL;\
			} \
			void DoUpdate(float dt); \
			void updateEntityComponent(float dt, Entity e, type##Component* t); \
		private: \
			type##System();	\
			static type##System* _instance;
#endif

#if USE_VECTOR_STORAGE
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
        for(std::map<Entity, type##Component*>::iterator it=components.begin(); it!=components.end();) { \
            Entity ent = it->first; \
				++it;

    #define FOR_EACH_ENTITY_COMPONENT(type, ent, comp) \
        for(std::map<Entity, type##Component*>::iterator it=components.begin(); it!=components.end();) { \
            Entity ent = it->first;\
            type##Component* comp = it->second; \
				++it;

    #define FOR_EACH_COMPONENT(type, comp) \
        for(std::map<Entity, type##Component*>::iterator it=components.begin(); it!=components.end();) { \
            type##Component* comp = it->second; \
				++it;


    #define FOR_EACH_EXT_ENTITY(type, ent) \
        std::vector<Entity> type##v = type##System::GetInstance().RetrieveAllEntityWithComponent(); \
        for(std::vector<Entity>::iterator it=type##v.begin(); it!=type##v.end();) { \
            Entity ent = *it; \
				++it;


    #define FOR_EACH_EXT_ENTITY_COMPONENT(type, ent, comp) \
        std::vector<Entity> type##v = type##System::GetInstance().RetrieveAllEntityWithComponent(); \
        for(std::vector<Entity>::iterator it=type##v.begin(); it!=type##v.end();) { \
            Entity ent = *it;\
				type##Component* comp = the##type##System.Get(*it); \
				++it;

    #define FOR_EACH_EXT_COMPONENT(type, comp) \
        std::vector<Entity> type##v = type##System::GetInstance().RetrieveAllEntityWithComponent(); \
        for(std::vector<Entity>::iterator it=type##v.begin(); it!=type##v.end();) { \
				type##Component* comp = the##type##System.Get(*it); \
				++it;

#endif

class ComponentSystem {
	public:
		ComponentSystem(const std::string& n) : name(n) { bool inserted = registry.insert(std::make_pair(name, this)).second; assert(inserted); }
        virtual ~ComponentSystem() { registry.erase(name); }

		virtual const std::string& getName() const { return name; }
		virtual void Add(Entity entity) = 0;
		virtual void Delete(Entity entity) = 0;
        virtual uint8_t* saveComponent(Entity entity, uint8_t* out = 0) = 0;
		virtual int serialize(Entity entity, uint8_t** out, void* ref = 0) = 0;
		virtual int deserialize(Entity entity, uint8_t* out, int size) = 0;

		void Update(float dt) {
            PROFILE("SystemUpdate", name, BeginEvent);
            #ifdef DEBUG
            float before = TimeUtil::GetTime();
            #endif
            DoUpdate(dt);
            #ifdef DEBUG
            updateDuration = TimeUtil::GetTime() - before;
            #endif
            PROFILE("SystemUpdate", name, EndEvent);
        }

		static ComponentSystem* Named(const std::string& n) {
			std::map<std::string, ComponentSystem*>::iterator it = registry.find(n);
			if (it == registry.end()) {
                LOG(ERROR) << "System with name: '" << n << "' does not exist";
				return 0;
			}
			return (*it).second;
		}

		static std::vector<std::string> registeredSystemNames();

#ifdef INGAME_EDITORS
        virtual void addEntityPropertiesToBar(Entity , TwBar* ) {}
#endif

	protected:
		virtual void DoUpdate(float dt) = 0;
		static std::map<std::string, ComponentSystem*> registry;
	private:
		std::string name;
    public:
        #ifdef DEBUG
        float updateDuration;
        #endif
};

template <typename T>
class ComponentSystemImpl: public ComponentSystem {
	public:
		ComponentSystemImpl(const std::string& t) : ComponentSystem(t) {
            previous = 0;
            #if USE_VECTOR_STORAGE
            _freelist = UINT_MAX;
            #endif
		}

		void Add(Entity entity) {
            #if USE_VECTOR_STORAGE
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
            assert(components.size() == entityWithComponent.size());
            #else
			assert (components.find(entity) == components.end());
			components.insert(std::make_pair(entity, CreateComponent()));
            #endif
		}

		#ifdef DEBUG
		virtual void preDeletionCheck(Entity) { };
		#endif

		void Delete(Entity entity) {
		#ifdef DEBUG
		    preDeletionCheck(entity);
		#endif
            #if USE_VECTOR_STORAGE
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

		T* Get(Entity entity, bool failIfNotfound = true) {
            if (entity != previous) {
                #if USE_VECTOR_STORAGE
                std::map<Entity, unsigned>::iterator it = entityToIndice.find(entity);
                if (it == entityToIndice.end()) {
                    if (failIfNotfound) {
                        LOG(ERROR) << "Entity '" << entity << "' has no component of type " << getName();
                        assert (false);
                    }
                    return 0;
                }
                previousComp = &components[it->second];
                #else
    			ComponentIt it = components.find(entity);
    			if (it == components.end()) {
                    #ifndef ANDROID
                    // crash here
                    if (failIfNotfound) {
                        #ifdef DEBUG
                        LOG(FATAL) << "Entity '" << theEntityManager.entityName(entity)
                            << "' (" << entity << ") has no component of type '" << getName();
                        #else
                        LOG(FATAL) << "Entity '" << entity << "' has no component of type '" << getName();
                        #endif
                    }
                    #endif
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
            #ifdef USE_VECTOR_STORAGE
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

		int deserialize(Entity entity, uint8_t* in, int size) {
			T* component = Get(entity, false);
            if (!component)
            #if USE_VECTOR_STORAGE
                { Add(entity); component = Get(entity); }
            #else
                component = new T();
            #endif
            int s = componentSerializer.deserializeObject(in, size, component);
            #if USE_VECTOR_STORAGE

            #else
			components[entity] = component;
            #endif
            return s;
		}

	protected:
        virtual T* CreateComponent() {
            return new T();
        }

#if USE_VECTOR_STORAGE
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
#ifdef DEBUG
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
        Serializer componentSerializer;
};
