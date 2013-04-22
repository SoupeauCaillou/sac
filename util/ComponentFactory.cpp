#include "ComponentFactory.h"
#include "DataFileParser.h"
#include "Serializer.h"
#include "base/PlacementHelper.h"
#include "base/Interval.h"
#include "systems/RenderingSystem.h"

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
    uint32_t html;
    // 0xffffffff variant
    // ....todo
    uint8_t rgba[4];
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
    #define LOAD_SINGLE(_type_) load(dfp, section, name, IntervalAsRandom, TYPE_2_PTR(_type_))
    #define LOAD_INTERVAL(_type_) { \
        Interval<_type_>* itv = TYPE_2_PTR(Interval<_type_>); \
        bool success = load(dfp, section, name, IntervalValue1, &itv->t1); \
        success &= load(dfp, section, name, IntervalValue2, &itv->t2); \
        count += success; }
    #define LOAD(_type_) if ((*it)->isInterval()) LOAD_INTERVAL(_type_) else count += LOAD_SINGLE(_type_);

    int count = 0;

	// Browse properties
    for (auto it = properties.begin(); it!=properties.end(); ++it) {
        // Retrieve property name
        const std::string& name = (*it)->getName();

        switch ((*it)->getType()) {
            case PropertyType::Float:
                LOAD(float);
                break;
            case PropertyType::Int:
                LOAD(int);
                break;
            case PropertyType::Vec2:
                LOAD(glm::vec2);
                break;
            case PropertyType::String:
                count += LOAD_SINGLE(std::string);
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
}
