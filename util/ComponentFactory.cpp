#include "ComponentFactory.h"
#include "DataFileParser.h"
#include "Serializer.h"
#include "base/PlacementHelper.h"
#include "base/Interval.h"
#include "systems/RenderingSystem.h"

const std::string vec2modifiers[] =
    { "", "%screen", "%screen_rev", "%screen_w", "%screen_h" };

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

template <class T>
int  load(const DataFileParser& dfp, const std::string& section, const std::string& name, T* out);


template <>
inline int load(const DataFileParser& dfp, const std::string& section, const std::string& name, glm::vec2* out) {
    float parsed[4];

    // 5 different variants


    for (int i = 0; i<5; i++) {
        if (dfp.get(section, name + vec2modifiers[i], parsed, 4, false)) {
            // we got an interval
            Interval<glm::vec2> itv(glm::vec2(parsed[0], parsed[1]), glm::vec2(parsed[2], parsed[3]));
            *out = itv.random();
            applyVec2Modifiers(i, out);
            LOG_SUCCESS_ << out->x << ", " << out->y << "'")
            return 1;
        } else if (dfp.get(section, name + vec2modifiers[i], parsed, 2, false)) {
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
inline int load(const DataFileParser& dfp, const std::string& section, const std::string& name, std::string* out) {
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
inline int  load(const DataFileParser& dfp, const std::string& section, const std::string& name, T* out) {
    T parsed[2];

    if (dfp.get(section, name, parsed, 2, false)) {
        // we got an interval
        Interval<T> itv(parsed[0], parsed[1]);
        *out = itv.random();
        LOG_SUCCESS
        return 1;
    } else if (dfp.get(section, name, parsed, 1, false)) {
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
    #define TYPE_2_PTR(_type_) (_type_ )((uint8_t*)component + (*it)->offset)
    #define LOAD(_type_) load(dfp, section, name, TYPE_2_PTR(_type_))

    int count = 0;

	// Browse properties
    for (auto it = properties.begin(); it!=properties.end(); ++it) {
        // Retrieve property name
        const std::string& name = (*it)->getName();

        switch ((*it)->getType()) {
            case PropertyType::Float:
                count += LOAD(float*);
                break;
            case PropertyType::Int:
                count += LOAD(int*);
                break;
            case PropertyType::Vec2:
                count += LOAD(glm::vec2*);
                break;
            case PropertyType::String:
                count += LOAD(std::string*);
                break;
            case PropertyType::Texture: {
                std::string textureName;
                if (load(dfp, section, name, &textureName)) {
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
