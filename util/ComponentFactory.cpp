#include "ComponentFactory.h"
#include "DataFileParser.h"
#include "Serializer.h"
#include "base/PlacementHelper.h"
#include "base/Interval.h"
#include "base/EntityManager.h"
#include "base/Entity.h"
#include "base/Log.h"
#include "systems/RenderingSystem.h"
#include "systems/opengl/EntityTemplateLibrary.h"
#include "systems/TransformationSystem.h"
#include "systems/AnchorSystem.h"

const std::string vec2modifiers[] =
    { "", "%screen", "%screen_rev", "%screen_w", "%screen_h" };

const std::string vec2singlefloatmodifiers[] = {
    "%texture_ratio,screen_w",
    "%texture_ratio,screen_h",
    "%screen_w,texture_ratio",
    "%screen_h,texture_ratio",
    "%texture_ratio,abs",
    "%abs,texture_ratio",
};
const std::string colormodifiers[] =
    { "", "%html", "%255", "%name" };

static void applyVec2Modifiers(int idx, glm::vec2* out) {
    switch (idx) {
        case 0:
            break;
        case 1:
            out->x *= PlacementHelper::ScreenWidth;
            out->y *= PlacementHelper::ScreenHeight;
            break;
        case 2:
            out->y *= PlacementHelper::ScreenWidth;
            out->x *= PlacementHelper::ScreenHeight;
            break;
        case 3:
            out->x *= PlacementHelper::ScreenWidth;
            out->y *= PlacementHelper::ScreenWidth;
            break;
        case 4:
            out->x *= PlacementHelper::ScreenHeight;
            out->y *= PlacementHelper::ScreenHeight;
            break;
    }
}

#define LOG_SUCCESS LOGV(2, "Loaded " << section << "/" << name << " property: '" << *out << "'");
#define LOG_SUCCESS_ LOGV(2, "Loaded " << section << "/" << name << " property: '"

enum IntervalMode {
    IntervalAsRandom,
    IntervalValue1,
    IntervalValue2,
};

template <class T>
int  load(const DataFileParser& dfp, const std::string& section, const std::string& name, IntervalMode mode, T* out);


template <>
inline int load(const DataFileParser& dfp, const std::string& section, const std::string& name, IntervalMode mode, glm::vec2* out) {
    float parsed[4];

    // 5 different variants
    for (int i = 0; i<5; i++) {
        if (dfp.get(section, name + vec2modifiers[i], parsed, 4, false)) {
            // we got an interval
            Interval<glm::vec2> itv(glm::vec2(parsed[0], parsed[1]), glm::vec2(parsed[2], parsed[3]));
            switch (mode) {
                case IntervalAsRandom: *out = itv.random(); break;
                case IntervalValue1: *out = itv.t1; break;
                case IntervalValue2: *out = itv.t2; break;
            }
            applyVec2Modifiers(i, out);
            LOG_SUCCESS_ << out->x << ", " << out->y << "'");
            return 1;
        } else if (mode == IntervalAsRandom && dfp.get(section, name + vec2modifiers[i], parsed, 2, false)) {
            // we got a single value
            *out = glm::vec2(parsed[0], parsed[1]);
            LOG_SUCCESS_ << out->x << ", " << out->y << "'");
            applyVec2Modifiers(i, out);
            return 1;
        }
    }

    // Try 6 variants with 1 float
    for (int i=0; i<6; i++) {
        if (dfp.get(section, name + vec2singlefloatmodifiers[i], parsed, 1, false)) {
            // hum oh
            std::string textureName;
            if (dfp.get("Rendering", "texture", &textureName, 1, false)) {
                const glm::vec2& s = theRenderingSystem.getTextureSize(textureName);
                switch (i) {
                    case 0:
                        out->y = parsed[0] * PlacementHelper::ScreenWidth;
                        out->x = out->y * s.x / s.y;
                        break;
                    case 1:
                        out->y = parsed[0] * PlacementHelper::ScreenHeight;
                        out->x = out->y * s.x / s.y;
                        break;
                    case 2:
                        out->x = parsed[0] * PlacementHelper::ScreenWidth;
                        out->y = out->x * s.y / s.x;
                        break;
                    case 3:
                        out->x = parsed[0] * PlacementHelper::ScreenHeight;
                        out->y = out->x * s.y / s.x;
                        break;
                    case 4:
                        out->y = parsed[0];
                        out->x = out->y * s.x / s.y;
                        break;
                    case 5:
                        out->x = parsed[0];
                        out->y = out->x * s.y / s.x;
                        break;
                }
                return 1;
            }
        }
    }

    return 0;
}

template <>
inline int load(const DataFileParser& dfp, const std::string& section, const std::string& name, IntervalMode mode, Color* out) {
    float p[8];
    // 3 different variants: first 4 float (or 8 for an interval)
    if (dfp.get(section, name, p, 8, false)) {
        // we got an interval
        Interval<Color> itv(Color(&p[0], 0xffffffff), Color(&p[4], 0xffffffff));
        switch (mode) {
            case IntervalAsRandom: *out = itv.random(); break;
            case IntervalValue1: *out = itv.t1; break;
            case IntervalValue2: *out = itv.t2; break;
        }
        LOG_SUCCESS_ << *out << "'");
        return 1;
    } else if (mode == IntervalAsRandom && dfp.get(section, name, p, 4, false)) {
        // we got a single value
        *out = Color(&p[0], 0xffffffff);
        LOG_SUCCESS_ << *out << "'");
        return 1;
    }
    std::string html;
    if (dfp.get(section, name + colormodifiers[1], &html, 1, false)) {
        int32_t h;
        std::istringstream iss(html);
        iss >> std::hex >> h;
        *out = Color(((h >> 16) & 0xff) / 255.0
            , ((h >> 8) & 0xff) / 255.0
            , ((h >> 0) & 0xff) / 255.0
            , 1);
        LOG_SUCCESS_ << *out << "'");
        return 1;
    }
    std::string tmp;
    if (dfp.get(section, name + colormodifiers[3], &tmp, 1, false)) {
        *out = Color(tmp);
        LOG_SUCCESS_ << *out << "'");
        return 1;
    }
    //;
    // 0xffffffff variant
    // ....todo
    //uint8_t rgba[4];
    // 128, 255, 0, 128 variant
    // ... todo
    return 0;
}

template <>
inline int load(const DataFileParser& dfp, const std::string& section, const std::string& name, IntervalMode, std::string* out) {
    std::string parsed;

    if (dfp.get(section, name, &parsed, 1, false)) {
        // we got a single value
        *out = parsed;
        LOG_SUCCESS
        return 1;
    } else {
        // fail
        return 0;
    }
}

template <class T>
inline int  load(const DataFileParser& dfp, const std::string& section, const std::string& name, IntervalMode mode, T* out) {
    T parsed[2];

    if (dfp.get(section, name, parsed, 2, false)) {
        // we got an interval
        Interval<T> itv(parsed[0], parsed[1]);
        switch (mode) {
            case IntervalAsRandom: *out = itv.random(); break;
            case IntervalValue1: *out = itv.t1; break;
            case IntervalValue2: *out = itv.t2; break;
        }
        LOG_SUCCESS
        return 1;
    } else if (mode == IntervalAsRandom && dfp.get(section, name, parsed, 1, false)) {
        // we got a single value
        *out = parsed[0];
        LOG_SUCCESS
        return 1;
    } else {
        // fail
        return 0;
    }
}

static bool loadSingleProperty(const std::string& context,
        const DataFileParser& dfp,
        const std::string& section,
        const std::string& name,
        PropertyType::Enum type,
        PropertyAttribute::Enum attr,
        PropertyNameValueMap& propMap,
        std::vector<std::string>& subEntities);

int ComponentFactory::build(
        const std::string& context,
        const DataFileParser& dfp,
        const std::string& section,
        const std::vector<IProperty*>& properties, EntityTemplate& templ,
        std::vector<std::string>& subEntities) {

    // lookup ComponentSystem by its name
    std::string realSystemName(section);
    if (realSystemName.rfind('#') != std::string::npos) {
        realSystemName = realSystemName.substr(realSystemName.rfind('#') + 1);
    }
    ComponentSystem* systm = ComponentSystem::Named(realSystemName);
    LOGE_IF(!systm, "Missing system: '" << section << "'");
    if (!systm)
        return 0;

    PropertyNameValueMap& propMap = templ[systm];
    // Cleanup current values
    for (auto it : propMap) {
        delete it.second;
    }

    // Browse properties for the given system
    for (auto it = properties.begin(); it!=properties.end(); ++it) {
        // Retrieve property name
        const std::string& name = (*it)->getName();
        const auto type = (*it)->getType();

        // Try to load property from data
        bool success = loadSingleProperty(context, dfp, section, name, type, (*it)->getAttribute(), propMap, subEntities);

        // special testing case
        if (!success && name == "position") {
            const std::string v[] = { "NE", "N", "NW", "E", "W", "SW", "S", "SE"};
            for (unsigned i=0; i<8; i++) {
                if (loadSingleProperty(context, dfp, section, name + v[i], type, (*it)->getAttribute(), propMap, subEntities)) {
                    break;
                }
            }
        }
    }

    return propMap.size();
}

void ComponentFactory::applyTemplate(Entity entity, void* component, const PropertyNameValueMap& propValueMap, const std::vector<IProperty*>& properties) {
    #define TYPE_2_PTR(_type_) (_type_ * )((uint8_t*)component + prop->offset)
    #define ASSIGN(_type_) { \
        Interval< _type_ > itv; \
        memcpy(&itv, (*it).second, sizeof(itv)); \
        if (prop->getAttribute() == PropertyAttribute::Interval) \
            *(TYPE_2_PTR(Interval < _type_ > )) = itv; \
        else \
            *(TYPE_2_PTR(_type_)) = itv.random(); }

    int positionHackIndex = -1;
    for (IProperty* prop : properties) {
        const std::string& name = prop->getName();
        auto it = propValueMap.find(name);
        if (it == propValueMap.end()) {
            if (name == "position") {
                // special testing case
                const std::string v[] = { "NW", "N", "NE", "W", "E", "SW", "S", "SE"};
                for (unsigned i=0; i<8; i++) {
                    it = propValueMap.find(name + v[i]);
                    if (it != propValueMap.end()) {
                        positionHackIndex = i;
                        break;
                    }
                }
            }
            if (it == propValueMap.end())
                continue;
        }

        switch (prop->getType()) {
           case PropertyType::Float:
                ASSIGN(float);
                break;
            case PropertyType::Int:
                ASSIGN(int);
                break;
            case PropertyType::Bool:
                ASSIGN(bool);
                break;
            case PropertyType::Vec2:
                ASSIGN(glm::vec2);
                break;
            case PropertyType::String: {
                if (prop->getAttribute() == PropertyAttribute::Vector) {
                    std::vector<std::string>* out = TYPE_2_PTR(std::vector<std::string>);
                    VectorProperty<std::string> vp("dummy", 0);
                    vp.deserialize((*it).second, out);
                } else {
                    char tmp[1024];
                    unsigned l;
                    memcpy(&l, (*it).second, sizeof(int));
                    memcpy(tmp, (*it).second + sizeof(int), l);
                    tmp[l] = '\0';
                    std::string* s = TYPE_2_PTR(std::string);
                    *s = tmp;
                }
                break;
            }
            case PropertyType::Color:
                ASSIGN(Color);
                break;
            case PropertyType::Texture: {
                memcpy((uint8_t*)component + prop->offset, (*it).second, sizeof(TextureRef));
                break;
            }
            case PropertyType::Entity: {
                EntityTemplateRef r;
                memcpy(&r, (*it).second, sizeof(r));
                Entity* e = TYPE_2_PTR(Entity);
                if (*e) {
                    theEntityManager.DeleteEntity(*e);
                }
                *e = theEntityManager.CreateEntity("sub_" + prop->getName(), EntityType::Volatile, r);
                ANCHOR(*e)->parent = entity;
                break;
            }
            default:
                break;
        }
    }

    if (positionHackIndex >= 0) {
        // const std::string v[] = { "NW", "N", "NE", "W", "E", "SW", "S", "SE"};
        const glm::vec2 coeff[] = {
            glm::vec2(-0.5, 0.5) , glm::vec2(0, 0.5) , glm::vec2(0.5, 0.5),
            glm::vec2(-0.5, 0.0)                     , glm::vec2(0.5, 0.0),
            glm::vec2(-0.5, -0.5), glm::vec2(0, -0.5), glm::vec2(0.5, -0.5),
        };
        // find position
        for (IProperty* prop : properties) {
            const std::string& name = prop->getName();
            if (name == "position") {
                glm::vec2* position = TYPE_2_PTR(glm::vec2);
                *position =
                    AnchorSystem::adjustPositionWithAnchor(*position, TRANSFORM(entity)->size * coeff[positionHackIndex]);
                break;
            }
        }
    }
}

static bool loadSingleProperty(const std::string& context,
        const DataFileParser& dfp,
        const std::string& section,
        const std::string& name,
        PropertyType::Enum type,
        PropertyAttribute::Enum attr,
        PropertyNameValueMap& propMap,
        std::vector<std::string>& subEntities) {

    #define LOAD_INTERVAL_TEMPL(_type_) { \
        Interval<_type_> itv; \
        bool success = load(dfp, section, name, IntervalValue1, &itv.t1); \
        if(success) load(dfp, section, name, IntervalValue2, &itv.t2);\
        else { success = load(dfp, section, name, IntervalAsRandom, &itv.t1); itv.t2 = itv.t1; } \
        if (success) {\
        uint8_t* arr = new uint8_t[sizeof(itv)];\
        memcpy(arr, &itv, sizeof(itv));\
        propMap.insert(std::make_pair(name, arr)); return true; }}

    switch (type) {
        case PropertyType::Float:
            LOAD_INTERVAL_TEMPL(float);
            break;
        case PropertyType::Int:
            LOAD_INTERVAL_TEMPL(int);
            break;
        case PropertyType::Bool:
            LOAD_INTERVAL_TEMPL(bool);
            break;
        case PropertyType::Vec2:
            LOAD_INTERVAL_TEMPL(glm::vec2);
            break;
        case PropertyType::String: {
            std::string s;
            // let's try something simple here
            if (attr == PropertyAttribute::Vector) {
                int splits = dfp.getSubStringCount(section, name);
                if (splits > 0) {
                    std::string* all = new std::string[splits];
                    if (dfp.get(section, name, all, splits, true)) {
                        std::vector<std::string> a;
                        for (int i=0; i<splits; i++)
                            a.push_back(all[i]);

                        VectorProperty<std::string> vp("dummy", 0);
                        unsigned size = vp.size(&a);
                        uint8_t* arr = new uint8_t[size];
                        vp.serialize(arr, &a);
                        propMap.insert(std::make_pair(name, arr));
                    }
                    delete[] all;
                }
            } else {
                if (load(dfp, section, name, IntervalAsRandom, &s)) {
                    unsigned l = s.length();
                    uint8_t* arr = new uint8_t[sizeof(int) + l];
                    memcpy(arr, &l, sizeof(int));
                    memcpy(&arr[sizeof(int)], s.c_str(), l);
                    propMap.insert(std::make_pair(name, arr));
                }
            }
            break;
        }
        case PropertyType::Entity: {
            std::string s;
            if (load(dfp, section, name + "%template", IntervalAsRandom, &s)) {
                std::string subEntityName(context + std::string("#") + name);
                EntityTemplateRef r = MurmurHash::compute(subEntityName.c_str(), subEntityName.length());
                uint8_t* arr = new uint8_t[sizeof(r)];
                memcpy(arr, &r, sizeof(r));
                propMap.insert(std::make_pair(name, arr));
                subEntities.push_back(name);
                theEntityManager.entityTemplateLibrary.defineParent(r,
                    theEntityManager.entityTemplateLibrary.load(s));
            }
            break;
        }
        case PropertyType::Color:
            LOAD_INTERVAL_TEMPL(Color);
            break;
        case PropertyType::Texture: {
            std::string textureName;
            if (load(dfp, section, name, IntervalAsRandom,&textureName)) {
                uint8_t* arr = new uint8_t[sizeof(TextureRef)];
                *((TextureRef*)arr) = theRenderingSystem.loadTextureFile(textureName);
                propMap.insert(std::make_pair(name, arr));
            }
            break;
        }
        default:
            LOGW("Unhandled property type: " << type << " for '" << name << "'");
            break;
    }
    #undef LOAD_INTERVAL_TEMPL
    return false;
}
