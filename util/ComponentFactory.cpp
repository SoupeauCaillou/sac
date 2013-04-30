#include "ComponentFactory.h"
#include "DataFileParser.h"
#include "Serializer.h"
#include "base/PlacementHelper.h"
#include "base/Interval.h"
#include "systems/RenderingSystem.h"
#include "systems/opengl/EntityTemplateLibrary.h"

const std::string vec2modifiers[] =
    { "", "%screen", "%screen_rev", "%screen_w", "%screen_h" };

const std::string colormodifiers[] =
    { "", "%html", "%255" };

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

#define LOG_SUCCESS LOGV(2, "Loaded " << section << "/" << name << " property: '" << *out << "'")
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
            LOG_SUCCESS_ << out->x << ", " << out->y << "'")
            return 1;
        } else if (mode == IntervalAsRandom && dfp.get(section, name + vec2modifiers[i], parsed, 2, false)) {
            // we got a single value
            *out = glm::vec2(parsed[0], parsed[1]);
            LOG_SUCCESS_ << out->x << ", " << out->y << "'")
            applyVec2Modifiers(i, out);
            return 1;
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
        LOG_SUCCESS_ << *out << "'")
        return 1;
    } else if (mode == IntervalAsRandom && dfp.get(section, name, p, 4, false)) {
        // we got a single value
        *out = Color(&p[0], 0xffffffff);
        LOG_SUCCESS_ << *out << "'")
        return 1;
    }
    //uint32_t html;
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

int ComponentFactory::build(const DataFileParser& dfp,
		const std::string& section,
		const std::vector<IProperty*>& properties, void* component) {
    #define TYPE_2_PTR(_type_) (_type_ * )((uint8_t*)component + (*it)->offset)
    #define LOAD_SINGLE(_type_) { \
        _type_ vvv; \
        if (load(dfp, section, name, IntervalAsRandom, &vvv)) {\
            *TYPE_2_PTR(_type_) = vvv; count++; }\
        }
    #define LOAD_INTERVAL(_type_) { \
        Interval<_type_> itv; \
        bool success = load(dfp, section, name, IntervalValue1, &itv.t1); \
        success &= load(dfp, section, name, IntervalValue2, &itv.t2); \
        count += success; if (success) *TYPE_2_PTR(Interval<_type_>) = itv;}
    #define LOAD(_type_) { if ((*it)->isInterval()) LOAD_INTERVAL(_type_) else LOAD_SINGLE(_type_) }

    int count = 0;

	// Browse properties
    for (auto it = properties.begin(); it!=properties.end(); ++it) {
        // Retrieve property name
        const std::string& name = (*it)->getName();

        switch ((*it)->getType()) {
            case PropertyType::Float:
                LOAD(float)
                break;
            case PropertyType::Int:
                LOAD(int)
                break;
            case PropertyType::Vec2:
                LOAD(glm::vec2)
                break;
            case PropertyType::String:
                LOAD_SINGLE(std::string)
                break;
            case PropertyType::Color:
                LOAD(Color);
                break;
            case PropertyType::Texture: {
                std::string textureName;
                if (load(dfp, section, name, IntervalAsRandom,&textureName)) {
                    count++;
                    *((TextureRef*)((uint8_t*)component + (*it)->offset)) = theRenderingSystem.loadTextureFile(textureName);
                }
                break;
            }
            default:
                break;
        }
    }
	return count;
    #undef TYPE_2_PTR
}

int ComponentFactory::build(const DataFileParser& dfp,
        const std::string& section,
        const std::vector<IProperty*>& properties, EntityTemplate& templ) {
    #define LOAD_INTERVAL_TEMPL(_type_) { \
        Interval<_type_> itv; \
        bool success = load(dfp, section, name, IntervalValue1, &itv.t1); \
        if(success) load(dfp, section, name, IntervalValue2, &itv.t2);\
        else { success = load(dfp, section, name, IntervalAsRandom, &itv.t1); itv.t2 = itv.t1; } \
        if (success) {\
        uint8_t* arr = new uint8_t[sizeof(itv)];\
        memcpy(arr, &itv, sizeof(itv));\
        propMap.insert(std::make_pair(name, arr)); }}

    ComponentSystem* systm = ComponentSystem::Named(section);
    LOGE_IF(!systm, "Missing system: '" << section << "'")
    if (!systm)
        return 0;
    PropertyNameValueMap& propMap = templ[systm];
    for (auto it : propMap) {
        delete it.second;
    }

    // Browse properties
    for (auto it = properties.begin(); it!=properties.end(); ++it) {
        // Retrieve property name
        const std::string& name = (*it)->getName();

        switch ((*it)->getType()) {
            case PropertyType::Float:
                LOAD_INTERVAL_TEMPL(float);
                break;
            case PropertyType::Int:
                LOAD_INTERVAL_TEMPL(int);
                break;
            case PropertyType::Vec2:
                LOAD_INTERVAL_TEMPL(glm::vec2);
                break;
            case PropertyType::String: {
                std::string s;
                if (load(dfp, section, name, IntervalAsRandom, &s)) {
                    unsigned l = s.length();
                    uint8_t* arr = new uint8_t[sizeof(int) + l];
                    memcpy(arr, &l, sizeof(int));
                    memcpy(&arr[sizeof(int)], s.c_str(), l);
                    propMap.insert(std::make_pair(name, arr));
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
                break;
        }
    }

    return propMap.size();
}

void ComponentFactory::applyTemplate(void* component, const PropertyNameValueMap& propValueMap, const std::vector<IProperty*>& properties) {
    #define TYPE_2_PTR(_type_) (_type_ * )((uint8_t*)component + prop->offset)
    #define ASSIGN(_type_) { \
        Interval< _type_ > itv; \
        memcpy(&itv, (*it).second, sizeof(itv)); \
        *(TYPE_2_PTR(_type_)) = itv.random(); }

    for (IProperty* prop : properties) {
        auto it = propValueMap.find(prop->getName());
        if (it == propValueMap.end())
            continue;
        switch (prop->getType()) {
           case PropertyType::Float:
                ASSIGN(float);
                break;
            case PropertyType::Int:
                ASSIGN(int);
                break;
            case PropertyType::Vec2:
                ASSIGN(glm::vec2);
                break;
            case PropertyType::String: {
                char tmp[1024];
                unsigned l;
                memcpy(&l, (*it).second, sizeof(int));
                memcpy(tmp, (*it).second + sizeof(int), l);
                tmp[l] = '\0';
                std::string* s = TYPE_2_PTR(std::string);
                *s = tmp;
                break;
            }
            case PropertyType::Color:
                ASSIGN(Color);
                break;
            case PropertyType::Texture: {
                memcpy((uint8_t*)component + prop->offset, (*it).second, sizeof(TextureRef));
                break;
            }
            default:
                break;
        }
    }
}
