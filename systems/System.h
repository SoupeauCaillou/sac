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

        const std::string& getName() const { return name; }

        virtual void Add(Entity entity) = 0;
        virtual void Delete(Entity entity);
        virtual uint8_t* saveComponent(Entity entity, uint8_t* out = 0) = 0;
        virtual void* componentAsVoidPtr(Entity e) = 0;

        void applyEntityTemplate(Entity entity, const PropertyNameValueMap& propMap, LocalizeAPI* localizeAPI);
        int serialize(Entity entity, uint8_t** out, void* ref = 0);
        int deserialize(Entity entity, uint8_t* out, int size);
        void suspendEntity(Entity entity);
        void resumeEntity(Entity entity);
        unsigned entityCount() const;
        void forEachEntityDo(std::function<void(Entity)> func);
        const std::vector<Entity>& RetrieveAllEntityWithComponent() const;

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

        void* enlargeComponentsArray(void* array, size_t compSize, uint32_t* size, uint32_t requested);
        void addEntity(Entity e);
    protected:
        std::string name;
        std::vector<Entity> entityWithComponent;
        std::list<Entity> suspended;

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
            // enlargeComponentsArray expects a > 0 size. And as components
            // is null anyway we can do this
            componentsSize = defaultStorageSize;
            components = reinterpret_cast<T*>
                (enlargeComponentsArray(0, sizeof(T), &componentsSize, defaultStorageSize));
        }

        void Add(Entity entity) {
            LOGF_IF(
                std::find(entityWithComponent.begin(), entityWithComponent.end(), entity) != entityWithComponent.end()
                , "Entity '" << theEntityManager.entityName(entity) << "' has the same component('" << getName() << "') twice!");

            components = reinterpret_cast<T*>
                (enlargeComponentsArray(components, sizeof(T), &componentsSize, entity));

            LOGT_EVERY_N(1000, "Add a reset method to components");
            // until a reset method method, do it the naive way
            new (&components[entity]) T();
            // components[entity] = T();

            addEntity(entity);
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

        void forEachECDo(std::function<void(Entity, T*)> func) {
            for (Entity e: entityWithComponent) {
                func(e, &components[e]);
            }
        }

        void* componentAsVoidPtr(Entity e) {
            return Get(e, false);
        }

        uint8_t* saveComponent(Entity entity, uint8_t* out) {
            if (!out) {
                out = (uint8_t*)(new T);//new uint8_t[sizeof(T)];
            }
            T* t = (T*)out;
            *t = *Get(entity);
            return out;
        }

    protected:
        uint32_t componentsSize;
        T* components;
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
