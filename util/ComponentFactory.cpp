#include "ComponentFactory.h"
#include "DataFileParser.h"
#include "Serializer.h"
#include "base/Interval.h"
#include "systems/RenderingSystem.h"

template <class T>
int  load(const DataFileParser& dfp, const std::string& section, const std::string& name, T* out);


template <>
inline int load(const DataFileParser& dfp, const std::string& section, const std::string& name, glm::vec2* out) {
    float parsed[4];

    if (dfp.get(section, name, parsed, 4, false)) {
        // we got an interval
        Interval<glm::vec2> itv(glm::vec2(parsed[0], parsed[1]), glm::vec2(parsed[2], parsed[3]));
        *out = itv.random();
        return 1;
    } else if (dfp.get(section, name, parsed, 2, false)) {
        // we got a single value
        *out = glm::vec2(parsed[0], parsed[1]);
        return 1;
    } else {
        // fail
        return 0;
    }
}

template <>
inline int load(const DataFileParser& dfp, const std::string& section, const std::string& name, std::string* out) {
    std::string parsed;

    if (dfp.get(section, name, &parsed, 1, false)) {
        // we got a single value
        *out = parsed;
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
        return 1;
    } else if (dfp.get(section, name, parsed, 1, false)) {
        // we got a single value
        *out = parsed[0];
        return 1;
    } else {
        // fail
        return 0;
    }
}

int ComponentFactory::build(const DataFileParser& dfp,
		const std::string& section,
		const std::vector<IProperty*>& properties, void* component) {
    int count = 0;

	// Browse properties
	for (auto it = properties.begin(); it!=properties.end(); ++it) {
		// Retrieve property name
		const std::string& name = (*it)->getName();

        switch ((*it)->getType()) {
            case PropertyType::Float:
                count += load(dfp, section, name, (float*)((uint8_t*)component + (*it)->offset));
                break;
            case PropertyType::Int:
                count += load(dfp, section, name, (int*)((uint8_t*)component + (*it)->offset));
                break;
            case PropertyType::Vec2:
                count += load(dfp, section, name, (glm::vec2*)((uint8_t*)component + (*it)->offset));
                break;
            case PropertyType::String:
                count += load(dfp, section, name, (std::string*)((uint8_t*)component + (*it)->offset));
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
		// Search for available value


	}
	return count;
}
