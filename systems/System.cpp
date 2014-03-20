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

#if SAC_INGAME_EDITORS
#include "systems/opengl/TextureLibrary.h"
#endif

std::map<std::string, ComponentSystem*> ComponentSystem::registry;


ComponentSystem::ComponentSystem(const std::string& n) : name(n)
#if SAC_DEBUG
    , updateDuration(0)
#endif
{
    bool inserted = registry.insert(std::make_pair(name, this)).second;
    LOGF_IF(!inserted, "System with name '" << name << "' already exists");
}

ComponentSystem::~ComponentSystem() {
    registry.erase(name);
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

ComponentSystem* ComponentSystem::Named(const std::string& n) {
    std::map<std::string, ComponentSystem*>::iterator it = registry.find(n);
    if (it == registry.end()) {
        LOGE("System with name: '" << n << "' does not exist");
        return 0;
    }
    return (*it).second;
}

std::vector<std::string> ComponentSystem::registeredSystemNames() {
    std::vector<std::string> result;
    result.reserve(registry.size());
    for (std::map<std::string, ComponentSystem*>::iterator it=registry.begin();
        it!=registry.end();
        ++it) {
        result.push_back(it->first);
    }
    return result;
}

const std::map<std::string, ComponentSystem*>& ComponentSystem::registeredSystems() {
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
            return TW_TYPE_STDSTRING;
        case PropertyType::Float:
        case PropertyType::Vec2:
            return TW_TYPE_FLOAT;
        case PropertyType::Int:
            return TW_TYPE_INT32;
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

bool ComponentSystem::addEntityPropertiesToBar(Entity e, TwBar* bar) {
    uint8_t* comp = static_cast<uint8_t*> (componentAsVoidPtr(e));
    if (!comp)
        return false;
    if (componentSerializer.getProperties().empty())
        return false;

    const std::string& group = name;
    // Browse properties, and add them to the TwBar
    for(IProperty* prop: componentSerializer.getProperties()) {

        if (prop->getAttribute() == PropertyAttribute::Vector) {
            LOGT("PropertyAttribute::Vector unhandled");
            continue;
        }

        const std::string& vname = prop->getName();
        const bool itv = (prop->getAttribute() == PropertyAttribute::Interval);
        VarType::Enum vt = itv ? VarType::INTERVAL_1 : VarType::NORMAL;
        switch (prop->getType()) {
            case PropertyType::String:
            case PropertyType::Int:
            case PropertyType::Bool:
            case PropertyType::Color:
                TwAddVarRW(bar, varName(name, vname, vt).c_str(),
                    PropertyTypeToType(prop->getType()), comp + prop->offset, varParams(group, vname, vt).c_str());

                if (itv) {
                    int size = 0;
                    switch (prop->getType()) {
                        case PropertyType::Int: size = sizeof(int); break;
                        case PropertyType::Bool: size = sizeof(bool); break;
                        case PropertyType::Color: size = 4 * sizeof(float); break;
                        default: size = 0;
                    }
                    if (size > 0) {
                        TwAddVarRW(bar, varName(name, vname, VarType::INTERVAL_2).c_str(),
                            PropertyTypeToType(prop->getType()), comp + prop->offset + size, varParams(group, vname, VarType::INTERVAL_2).c_str());
                    }
                }
                break;
            case PropertyType::Float:
                TwAddVarRW(bar, varName(name, vname, vt).c_str(),
                    PropertyTypeToType(prop->getType()), comp + prop->offset, varParams(group, vname, vt, FLOAT_PROPERTIES).c_str());
                if (itv) {
                    TwAddVarRW(bar, varName(name, vname, VarType::INTERVAL_2).c_str(),
                        PropertyTypeToType(prop->getType()), comp + prop->offset + sizeof(float), varParams(group, vname, VarType::INTERVAL_2, FLOAT_PROPERTIES).c_str());
                }
                break;
            case PropertyType::Vec2:
                // x component
                TwAddVarRW(bar, varName(name, vname, VarType::VEC2_X).c_str(),
                    PropertyTypeToType(prop->getType()), comp + prop->offset, varParams(group, vname, VarType::VEC2_X, FLOAT_PROPERTIES).c_str());
                // y component
                TwAddVarRW(bar, varName(name, vname, VarType::VEC2_Y).c_str(),
                    PropertyTypeToType(prop->getType()), comp + prop->offset + sizeof(float), varParams(group, vname, VarType::VEC2_Y, FLOAT_PROPERTIES).c_str());
                break;
            case PropertyType::Texture:
                TwAddVarCB(bar, varName(name, vname).c_str(),
                    TW_TYPE_STDSTRING, (TwSetVarCallback)textureSetCB, (TwGetVarCallback)textureGetCB,
                    comp + prop->offset, varParams(group, vname).c_str());
                break;
            case PropertyType::Entity:
                TwAddVarCB(bar, varName(name, vname).c_str(),
                    TW_TYPE_STDSTRING, 0, (TwGetVarCallback)entityGetCB,
                    comp + prop->offset, varParams(group, vname).c_str());
                break;
            default:
                break;
        }
    }
    return true;
}

static void textureSetCB(const void* valueIn, void* clientData) {
    const std::string* s = static_cast<const std::string*> (valueIn);
    TextureRef* r = static_cast<TextureRef*>(clientData);
    LOGI("textureSetCB : '" << *s << "'");
    *r = theRenderingSystem.loadTextureFile(*s);
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

#endif
