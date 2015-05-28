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



#include "System.h"
#include "systems/RenderingSystem.h"
#include <stdlib.h>
#if SAC_INGAME_EDITORS
#include "systems/opengl/TextureLibrary.h"
#include "imgui.h"
#endif
#include "util/MurmurHash.h"

#include "util/ComponentFactory.h"
#include "base/TimeUtil.h"
#include "base/Profiler.h"
#include "util/SerializerProperty.h"
#include <algorithm>
#include "base/EntityManager.h"
    
std::map<hash_t, ComponentSystem*> ComponentSystem::registry;


bool ComponentSystem::entityHasComponent(const std::vector<Entity>& c, Entity e) {
    return std::binary_search(c.begin(), c.end(), e);
}

ComponentSystem::ComponentSystem(hash_t n) : type(ComponentType::POD), id(n)
#if SAC_DEBUG
    , updateDuration(0)
#endif
{
    bool inserted = registry.insert(std::make_pair(id, this)).second;
    LOGF_IF(!inserted, "System with name '" << INV_HASH(id) << "' already exists");
}

ComponentSystem::ComponentSystem(hash_t n, ComponentType::Enum t) : type(t), id(n)
#if SAC_DEBUG
    , updateDuration(0)
#endif
{
    bool inserted = registry.insert(std::make_pair(id, this)).second;
    LOGF_IF(!inserted, "System with name '" << INV_HASH(id) << "' already exists");
}

ComponentSystem::~ComponentSystem() {
    registry.erase(id);
}

void* ComponentSystem::enlargeComponentsArray(
    void* array, size_t compSize, uint32_t* size, uint32_t requested, bool freeOldStorage) {
    // make sure array is big enough

    LOGV(1, "Resizing storage of " << INV_HASH(id) << "System. Previously acquired components may be invalid");
    int newSize = glm::max(2 * (*size), requested);
    void* ptr = NULL;
    if (freeOldStorage) {
        ptr = realloc(array, newSize * compSize);
    } else {
        ptr = malloc(newSize * compSize);
    }
    if (array)
        array = ptr;
    (*size) = newSize;

    return ptr;
}

void ComponentSystem::addEntity(Entity entity) {
    // sorted insert
    auto it=entityWithComponent.begin();
    for (; it!=entityWithComponent.end(); ++it) {
        if (*it > entity)
            break;
    }
    entityWithComponent.insert(it, entity);
}

void ComponentSystem::Delete(Entity entity) {
    auto it = std::find(entityWithComponent.begin(), entityWithComponent.end(), entity);
    LOGF_IF(it == entityWithComponent.end(), "Unable to find entity '" << theEntityManager.entityName(entity) << "' in components '" << INV_HASH(getId()) << "'");
    entityWithComponent.erase(it);
}

void ComponentSystem::deleteAllEntities() {
    auto copy = entityWithComponent;
    for (auto e: copy) {
        theEntityManager.DeleteEntity(e);
    }
    LOGF_IF(!entityWithComponent.empty(), "Entity list should be empty after deleteAll");
}

unsigned ComponentSystem::entityCount() const {
    return entityWithComponent.size();
}



const std::vector<Entity>& ComponentSystem::RetrieveAllEntityWithComponent() const {
    return entityWithComponent;
}

void ComponentSystem::forEachEntityDo(std::function<void(Entity)> func) {
    for (Entity e: entityWithComponent) {
        func(e);
    }
}

int ComponentSystem::serialize(Entity entity, uint8_t** out, void* ref) {
    void* component = componentAsVoidPtr(entity);
    return componentSerializer.serializeObject(out, component, ref);
}

int ComponentSystem::deserialize(Entity entity, uint8_t* in, int size) {
    void* component = componentAsVoidPtr(entity);
    if (!component) {
        theEntityManager.AddComponent(entity, this, true);
        component = componentAsVoidPtr(entity);
    }
    int s = componentSerializer.deserializeObject(in, size, component);
    return s;
}

void ComponentSystem::applyEntityTemplate(Entity entity, const PropertyNameValueMap& propMap, LocalizeAPI* localizeAPI) {
    void* component = componentAsVoidPtr(entity);
    ComponentFactory::applyTemplate(entity, component, propMap, componentSerializer.getProperties(), localizeAPI);
}

void ComponentSystem::Update(float dt) {
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

ComponentSystem* ComponentSystem::GetById(hash_t id) {
    auto it = registry.find(id);
    if (it == registry.end()) {
        LOGE("System with id: '" << INV_HASH(id) << "' / " << id << " does not exist");
        return 0;
    }
    return (*it).second;
}

std::vector<hash_t> ComponentSystem::registeredSystemIds() {
    std::vector<hash_t> result;
    result.reserve(registry.size());
    for (auto it=registry.begin();
        it!=registry.end();
        ++it) {
        result.push_back(it->first);
    }
    return result;
}

const std::map<hash_t, ComponentSystem*>& ComponentSystem::registeredSystems() {
    return registry;
}

#if SAC_INGAME_EDITORS
#define FLOAT_PROPERTIES "precision=3 step=0.01"

namespace VarType {
    enum Enum {
        NORMAL,
        VEC2_X,
        VEC2_Y,
        INTERVAL_1,
        INTERVAL_2, // TODO: vec2 interval
    };
}

#if 0
static std::string varLabel(const std::string& name, VarType::Enum t) {
    switch (t) {
        case VarType::NORMAL:
            return name;
        case VarType::INTERVAL_1:
            return name + ".1";
        case VarType::INTERVAL_2:
            return name + ".2";
        case VarType::VEC2_X:
            return name + ".x";
        case VarType::VEC2_Y:
            return name + ".y";
        default:
            LOGF("Unhandled");
            return "";
    }
}
static std::string varName(const std::string& group, const std::string& name, VarType::Enum t = VarType::NORMAL) {
    return group + "/" + varLabel(name, t);
}

static std::string varParams(const std::string& group, const std::string& name, VarType::Enum t = VarType::NORMAL, const std::string& p = "") {
    std::stringstream params;
    params << "group=" << group << " label='" << varLabel(name, t) << "' " << p;
    return params.str();
}

static TwType PropertyTypeToType(PropertyType::Enum e) {
    switch (e) {
        case PropertyType::String:
        case PropertyType::Hash:
            return TW_TYPE_STDSTRING;
        case PropertyType::Float:
        case PropertyType::Vec2:
            return TW_TYPE_FLOAT;
        case PropertyType::Int:
            return TW_TYPE_INT32;
        case PropertyType::Int8:
            return TW_TYPE_INT8;
        case PropertyType::Color:
            return TW_TYPE_COLOR4F;
        case PropertyType::Bool:
            return TW_TYPE_BOOLCPP;
        default:
            return TW_TYPE_INT32;
    }
}

static void textureSetCB(const void* valueIn, void* clientData);
static void textureGetCB(void* valueOut, void* clientData);
static void entityGetCB(void* valueOut, void* clientData);
static void hashGetCB(void* valueOut, void* clientData);
#endif

static char** textures_list = NULL;
static int texture_count = 0;

static int updateTextureList(hash_t h, int* count) {
    int current = 0;
    const auto& ref = theRenderingSystem.textureLibrary.getAllIndexes();
    // compute size
    *count = ref.size() + 1; // + 1 == no texture
    textures_list = (char**) realloc(textures_list, (*count) * sizeof(char*));
    if ((*count) > texture_count)
        memset(&textures_list[texture_count], 0, sizeof(char*) * (*count - texture_count));
    texture_count = *count;

    textures_list[0] = (char*)"N/A";
    int i = 1;
    for (auto p: ref) {
        const char* n = INV_HASH(p.ref);
        // exclude typo texture
        if (strstr(n, "_typo") != NULL) {
            (*count) --;
            continue;
        }

        char* ptr = textures_list[i] = (char*) realloc(textures_list[i], strlen(n) + 1);
        strcpy(ptr, n);
        if (h == p.ref)
            current = i;
        i++;
    }
    return current;
}

bool ComponentSystem::saveEntityToFile(Entity e, FILE* file) {
    uint8_t* comp = static_cast<uint8_t*> (componentAsVoidPtr(e));
    if (!comp)
        return false;
    if (componentSerializer.getProperties().empty())
        return false;

    fprintf(file, "\n[%s]\n", INV_HASH(id));

    for(IProperty* prop: componentSerializer.getProperties()) {

        if (prop->getAttribute() == PropertyAttribute::Vector
            || prop->getAttribute() == PropertyAttribute::Interval) {
            LOGT_EVERY_N(1000, "PropertyAttribute::Vector unhandled");
            continue;
        }

        const char* vname = Murmur::lookup(prop->getId());
        fprintf(file, "%s = ", vname);

        switch (prop->getType()) {
            case PropertyType::String: {
                std::string *s = (std::string*)(comp + prop->offset);
                fprintf(file, "%s\n", s->c_str());
                break;
            }
            case PropertyType::Int: {
                fprintf(file, "%d\n", *(int*)(comp + prop->offset));
                break;
            }

            case PropertyType::Int8: {
                fprintf(file, "%d\n", *(int8_t*)(comp + prop->offset));
                break;
            }
            case PropertyType::Bool: {
                fprintf(file, "%d\n", *(bool*)(comp + prop->offset));
                break;
            }
            case PropertyType::Color: {
                Color* c = (Color*)(comp + prop->offset);
                fprintf(file, "%.3f, %.3f, %.3f, %.3f\n", c->r, c->g, c->b, c->a);
                break;
            }
            case PropertyType::Float:
                fprintf(file, "%.3f\n", *(float*)(comp + prop->offset));
                break;
            case PropertyType::Vec2: {
                // x component
                glm::vec2* v = (glm::vec2*)(comp + prop->offset);
                fprintf(file, "%.3f, %.3f\n", v->x, v->y);

                break;
            }
            case PropertyType::Entity:
                fprintf(file, "%s\n", theEntityManager.entityName(*(Entity*)(comp + prop->offset)));
                break;
            case PropertyType::Texture:
            case PropertyType::Hash: {
                hash_t* h = (hash_t*)(comp + prop->offset);
                if (*h == InvalidTextureRef) {
                    fprintf(file, "\n");
                    break;
                }
                fprintf(file, "%s\n", INV_HASH(*h));
                break;
            }
            default:
                LOGE("PropertyType " << prop->getType() << " not supported for saving");
                fprintf(file, "\r# not supported yet - %s\n", vname);
                break;
        }
    }
    return true;
}

bool ComponentSystem::addEntityPropertiesToBar(Entity e, void* /*bar*/) {
    uint8_t* comp = static_cast<uint8_t*> (componentAsVoidPtr(e));
    if (!comp)
        return false;
    if (componentSerializer.getProperties().empty())
        return false;

    const std::string& group = INV_HASH(id);

    if (!ImGui::CollapsingHeader(group.c_str()))
        return false;

    // Browse properties, and add them to the TwBar
    for(IProperty* prop: componentSerializer.getProperties()) {

        if (prop->getAttribute() == PropertyAttribute::Vector) {
            LOGT_EVERY_N(1000, "PropertyAttribute::Vector unhandled");
            continue;
        }

        const std::string& vname = Murmur::lookup(prop->getId());
        switch (prop->getType()) {
            case PropertyType::String: {
                std::string *s = (std::string*)(comp + prop->offset);
                ImGui::LabelText(vname.c_str(), s->c_str());
                break;
            }
            case PropertyType::Int: {
                ImGui::InputInt(vname.c_str(), (int*)(comp + prop->offset));
                break;
            }

            case PropertyType::Int8: {
                ImGui::Value(vname.c_str(), *(int8_t*)(comp + prop->offset));
                break;
            }
            case PropertyType::Bool: {
                ImGui::Checkbox(vname.c_str(), (bool*)(comp + prop->offset));
                break;
            }
            case PropertyType::Color: {
                Color* c = (Color*)(comp + prop->offset);
                ImGui::ColorEdit4(vname.c_str(), c->rgba);
                break;
            }
            case PropertyType::Float:
                ImGui::InputFloat(vname.c_str(), (float*)(comp + prop->offset));
                break;
            case PropertyType::Vec2:
                // x component
                ImGui::InputFloat2(vname.c_str(), (float*)(comp + prop->offset));
                break;
            case PropertyType::Entity:
                ImGui::LabelText(vname.c_str(), theEntityManager.entityName(*(Entity*)(comp + prop->offset)));
                break;
            case PropertyType::Texture: {
                hash_t* h = (hash_t*)(comp + prop->offset);
                /* list existing texture in a combobox */
                int count = 0;
                int current = updateTextureList(*h, &count);
                if (ImGui::Combo(vname.c_str(), &current, const_cast<const char**> (textures_list), count)) {
                    *h = Murmur::RuntimeHash(textures_list[current]);
                }

                // ImGui::LabelText(vname.c_str(), (h != -1) ? (INV_HASH(h)) : "-");
                break;
            }
            case PropertyType::Hash: {
                hash_t h = *(hash_t*)(comp + prop->offset);
                ImGui::LabelText(vname.c_str(), (h != 0) ? (INV_HASH(h)) : "-");
                break;
            }
            default:
                break;
        }
    }
    return true;
}

#if 0
static void textureSetCB(const void* valueIn, void* clientData) {
    const std::string* s = static_cast<const std::string*> (valueIn);
    TextureRef* r = static_cast<TextureRef*>(clientData);
    LOGI("textureSetCB : '" << *s << "'");
    *r = theRenderingSystem.loadTextureFile(s->c_str());
}

static void textureGetCB(void* valueOut, void* clientData) {
    const TextureRef* r = static_cast<const TextureRef*> (clientData);
    std::string* s = static_cast<std::string*>(valueOut);
    if (*r == InvalidTextureRef) {
        TwCopyStdStringToLibrary(*s, "no texture");
    } else {
        // see http://anttweakbar.sourceforge.net/doc/tools:anttweakbar:twcopystdstringtolibrary
        TwCopyStdStringToLibrary(*s, theRenderingSystem.textureLibrary.ref2Name(*r));
    }
}

static void entityGetCB(void* valueOut, void* clientData) {
    const Entity* r = static_cast<const Entity*> (clientData);
    std::string* s = static_cast<std::string*>(valueOut);
    if (*r <= 0) {
        TwCopyStdStringToLibrary(*s, "none");
    } else {
        // see http://anttweakbar.sourceforge.net/doc/tools:anttweakbar:twcopystdstringtolibrary
        std::stringstream a;
        a << theEntityManager.entityName(*r) << '_' << *r;
        TwCopyStdStringToLibrary(*s, a.str());
    }
}

static void hashGetCB(void* valueOut, void* clientData) {
    const hash_t* r = static_cast<const hash_t*> (clientData);
    const char* lk = Murmur::lookup(*r);
    std::string* s = static_cast<std::string*>(valueOut);
    // see http://anttweakbar.sourceforge.net/doc/tools:anttweakbar:twcopystdstringtolibrary
    TwCopyStdStringToLibrary(*s, std::string(lk));
}
#endif

#endif
