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

class ComponentSystem {
    public:
        ComponentSystem(const std::string& n) ;

        virtual ~ComponentSystem();

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

        void Update(float dt);

        static ComponentSystem* Named(const std::string& n);

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
        ComponentSystemImpl(const std::string& t, unsigned defaultStorageSize = 8) : ComponentSystem(t) {
            LOGF_IF(defaultStorageSize == 0, "Storage size must be > 0");
            components.resize(defaultStorageSize);
        }

        void Add(Entity entity) {
            LOGF_IF(
                std::find(entityWithComponent.begin(), entityWithComponent.end(), entity) != entityWithComponent.end()
                , "Entity '" << theEntityManager.entityName(entity) << "' has the same component('" << getName() << "') twice!");

            // make sure array is big enough
            while (components.size() <= entity) {
                LOGV(1, "Resizing storage of " << name << "System. Previously acquired " << name << "Component* may be invalid");
                components.resize(2 * components.size());
            }
            LOGT_EVERY_N(1000, "Add a reset method to components");
            // until a reset method method, do it the naive way
            components[entity] = T();

            // sorted insert
            auto it=entityWithComponent.begin();
            for (; it!=entityWithComponent.end(); ++it) {
                if (*it > entity)
                    break;
            }
            entityWithComponent.insert(it, entity);
        }

        virtual void Delete(Entity entity) {
            auto it = std::find(entityWithComponent.begin(), entityWithComponent.end(), entity);
            LOGF_IF(it == entityWithComponent.end(), "Unable to find entity '" << theEntityManager.entityName(entity) << "' in components '" << getName() << "'");
            entityWithComponent.erase(it);
        }

        void suspendEntity(Entity entity) {
            auto it = std::find(entityWithComponent.begin(), entityWithComponent.end(), entity);
            LOGF_IF(it == entityWithComponent.end(), "Suspending an invalid entity " << entity);
            entityWithComponent.erase(it);
            suspended.push_back(entity);
        }

        void resumeEntity(Entity entity) {
            auto st = std::find(suspended.begin(), suspended.end(), entity);
            LOGF_IF(st == suspended.end(), "Resuming a not suspended entity " << entity);
            suspended.erase(st);

            // sorted insert
            auto it=entityWithComponent.begin();
            for (; it!=entityWithComponent.end(); ++it) {
                if (*it > entity)
                    break;
            }
            entityWithComponent.insert(it, entity);
        }

        unsigned entityCount() const {
            return entityWithComponent.size();
        }

        void* componentAsVoidPtr(Entity e) {
            return Get(e, false);
        }

#if SAC_DEBUG
        T* Get(Entity entity, bool failIfNotfound = true,
            const char* file = "\0", int line = 0) {

            theEntityManager.validateEntity(entity);
#else
        T* Get(Entity entity, bool failIfNotfound = true,
            const char* LOG_USAGE_ONLY(file) = 0, int LOG_USAGE_ONLY(line) = 0) {
#endif

            bool check =
#if SAC_DEBUG
            // always check in debug
                true;
#else
            // in release only check if call expect a nullptr in case of failure
                !failIfNotfound;
#endif
            if (check) {
                if (!std::binary_search(entityWithComponent.begin(), entityWithComponent.end(), entity)) {
                    if (failIfNotfound) {
                        LOGF("Entity '" << theEntityManager.entityName(entity)
                            << "' (" << entity << ") has no component of type '" << getName() << "' [@ " << file << ':' << line << ']');
                    }

                    return 0;
                }
            }
            return &components[entity];
        }


        const std::vector<Entity>&
         RetrieveAllEntityWithComponent() {
            return entityWithComponent;
        }

        void forEachEntityDo(std::function<void(Entity)> func) {
            for (Entity e: entityWithComponent) {
                func(e);
            }
        }

        void forEachECDo(std::function<void(Entity, T*)> func) {
            for (Entity e: entityWithComponent) {
                func(e, &components[e]);
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
        std::vector<Entity> entityWithComponent;
        std::list<Entity> suspended;
        std::vector<T> components;
};


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

#define FOR_EACH_COMPONENT(type, comp) \
    for (auto ________ent: entityWithComponent) { \
        auto* comp = &components[________ent];

#define FOR_EACH_ENTITY_COMPONENT(type, ent, comp) \
    for (auto ent: entityWithComponent) { \
        auto* comp = &components[ent];

//this macro is used to avoid IDE highlighting problems with brace missing...
#define END_FOR_EACH() }
