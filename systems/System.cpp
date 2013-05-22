#include "System.h"
#include <sstream>
#include "systems/RenderingSystem.h"

std::map<std::string, ComponentSystem*> ComponentSystem::registry;

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
#define FLOAT_PROPERTIES "precision=3 step=0,01"

namespace VarType {
    enum Enum {
        NORMAL,
        VEC2_X,
        VEC2_Y
    };
}

static std::string varLabel(const std::string& name, VarType::Enum t) {
    switch (t) {
        case VarType::NORMAL:
            return name;
        case VarType::VEC2_X:
            return name + ".x";
        case VarType::VEC2_Y:
            return name + ".y";
        default:
            LOGF("Unhandled");
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

void ComponentSystem::addEntityPropertiesToBar(Entity e, TwBar* bar) {
    uint8_t* comp = static_cast<uint8_t*> (componentAsVoidPtr(e));
    if (!comp)
        return;
    const std::string& group = name;
    // Browse properties, and add them to the TwBar
    for(IProperty* prop: componentSerializer.getProperties()) {
        const std::string& propName = prop->getName();

        if (prop->getAttribute() != PropertyAttribute::None) {
            LOGT("Unhandled property attributes");
            continue;
        }

        const std::string& vname = prop->getName();
        switch (prop->getType()) {
            case PropertyType::String:
            case PropertyType::Int:
            case PropertyType::Bool:
            case PropertyType::Color:
                TwAddVarRW(bar, varName(name, vname).c_str(),
                    PropertyTypeToType(prop->getType()), comp + prop->offset, varParams(group, vname).c_str());
                break;
            case PropertyType::Float:
                TwAddVarRW(bar, varName(name, vname).c_str(),
                    PropertyTypeToType(prop->getType()), comp + prop->offset, varParams(group, vname, VarType::NORMAL, FLOAT_PROPERTIES).c_str());
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
                    TW_TYPE_STDSTRING, textureSetCB, textureGetCB, comp + prop->offset, varParams(group, vname).c_str());
                break;
            case PropertyType::Entity:
                TwAddVarCB(bar, varName(name, vname).c_str(),
                    TW_TYPE_STDSTRING, 0, entityGetCB, comp + prop->offset, varParams(group, vname).c_str());
                break;
                /*
        Vec2,
        Int,
        Float,
        Color,
        Texture,
        Entity,
        */
        }
    }
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
        TwCopyStdStringToLibrary(*s, theRenderingSystem.textureLibrary.reverse(*r));
    }
}

static void entityGetCB(void* valueOut, void* clientData) {
    const Entity* r = static_cast<const Entity*> (clientData);
    std::string* s = static_cast<std::string*>(valueOut);
    if (*r <= 0) {
        TwCopyStdStringToLibrary(*s, "none");
    } else {
        // see http://anttweakbar.sourceforge.net/doc/tools:anttweakbar:twcopystdstringtolibrary
        TwCopyStdStringToLibrary(*s, theEntityManager.entityName(*r));
    }
}

#endif
