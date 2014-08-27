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



#include "ComponentFactory.h"
#include "DataFileParser.h"
#include "Serializer.h"
#include "api/LocalizeAPI.h"
#include "base/PlacementHelper.h"
#include "base/Interval.h"
#include "base/EntityManager.h"
#include "base/Entity.h"
#include "base/Log.h"
#include "systems/RenderingSystem.h"
#include "systems/SoundSystem.h"
#include "systems/opengl/EntityTemplateLibrary.h"
#include "systems/TransformationSystem.h"
#include "systems/AnchorSystem.h"
#include "util/MurmurHash.h"

#include <glm/glm.hpp>

#include <alloca.h>
#include <string>

static constexpr hash_t positionHack[] = {
    Murmur::_Hash("positionNW"),
    Murmur::_Hash("positionN"),
    Murmur::_Hash("positionNE"),
    Murmur::_Hash("positionW"),
    Murmur::_Hash("positionE"),
    Murmur::_Hash("positionSW"),
    Murmur::_Hash("positionS"),
    Murmur::_Hash("positionSE")
};

const char* stringmodifiers[] =
    { "%loc" };

static void applyVec2Modifiers(const hash_t mod, glm::vec2* out, int count) {
    for (int i=0; i<count; i++) {
        if (mod == 0) {
            continue;
        } else if (mod == HASH("screen", 0xb548a1f1)) {
            out[i] *= PlacementHelper::ScreenSize;
            continue;
        } else if (mod == HASH("screen_rev", 0x35ba1812)) {
            out[i].y *= PlacementHelper::ScreenSize.x;
            out[i].x *= PlacementHelper::ScreenSize.y;
            continue;
        } else if (mod == HASH("screen_w", 0xd120837a)) {
            out[i].x *= PlacementHelper::ScreenSize.x;
            out[i].y *= PlacementHelper::ScreenSize.x;
            continue;
        } else if (mod == HASH("screen_h", 0x14bde53f)) {
            out[i].x *= PlacementHelper::ScreenSize.y;
            out[i].y *= PlacementHelper::ScreenSize.y;
            continue;
        } else if (mod == HASH("gimp_size", 0x9ff0e717)) {
            out[i].x = PlacementHelper::GimpWidthToScreen(out[i].x);
            out[i].y = PlacementHelper::GimpHeightToScreen(out[i].y);
            continue;
        } else if (mod == HASH("gimp_pos", 0x6a491150)) {
            out[i].x = PlacementHelper::GimpXToScreen(out[i].x);
            out[i].y = PlacementHelper::GimpYToScreen(out[i].y);
            continue;
        } else {
            LOGF("Unknown vec2 modifier: '" << INV_HASH(mod) << "' (" << mod << ')');
        }
    }
}

static void applyFloatModifiers(const hash_t modifier, float* out, int count) {
    for (int i=0; i<count; i++) {
        if (modifier == 0) {
            continue;
        } else if (modifier == HASH("screen_w", 0xd120837a)) {
            out[i] *= PlacementHelper::ScreenSize.x;
            continue;
        } else if (modifier == HASH("screen_h", 0x14bde53f)) {
            out[i] *= PlacementHelper::ScreenSize.y;
            continue;
        } else if (modifier == HASH("gimp_x", 0x56c07e3e)) {
            out[i] = PlacementHelper::GimpXToScreen(out[i]);
            continue;
        } else if (modifier == HASH("gimp_y", 0x54c5ec09)) {
            out[i] = PlacementHelper::GimpYToScreen(out[i]);
            continue;
        } else if (modifier == HASH("gimp_w", 0x2b66a994)) {
            out[i] = PlacementHelper::GimpWidthToScreen(out[i]);
            continue;
        } else if (modifier == HASH("gimp_h", 0x42ba0dc3)) {
            out[i] = PlacementHelper::GimpHeightToScreen(out[i]);
            continue;
        } else if (modifier == HASH("degrees", 0x8e6c8522)) {
            out[i] = glm::radians(out[i]);
            continue;
        } else {
            LOGE("Unknown float modifier '" << INV_HASH(modifier) << "' (" << modifier << ')');
        }
    }
}


static void applyVec2SingleFloatModifiers(const hash_t modifier, const glm::vec2& textureSize, float in, glm::vec2* out) {
    if (modifier == HASH("texture_ratio,screen_w", 0x70a49fb4)) {
        out->y = in * PlacementHelper::ScreenSize.x;
        out->x = out->y * textureSize.x / textureSize.y;
    } else if (modifier == HASH("texture_ratio,screen_h", 0x985b8e11)) {
        out->y = in * PlacementHelper::ScreenSize.y;
        out->x = out->y * textureSize.x / textureSize.y;
    } else if (modifier == HASH("screen_w,texture_ratio", 0xa2fd11b3)) {
        out->x = in * PlacementHelper::ScreenSize.x;
        out->y = out->x * textureSize.y / textureSize.x;
    } else if (modifier == HASH("screen_h,texture_ratio", 0xee7d8330)) {
        out->x = in * PlacementHelper::ScreenSize.y;
        out->y = out->x * textureSize.y / textureSize.x;
    } else if (modifier == HASH("texture_ratio,abs", 0x433820f5)) {
        out->y = in;
        out->x = out->y * textureSize.x / textureSize.y;
    } else if (modifier == HASH("abs,texture_ratio", 0xf72162fb)) {
        out->x = in;
        out->y = out->x * textureSize.y / textureSize.x;
    } else if (modifier == HASH("texture", 0x3d4e3ff8)) {
        *out = PlacementHelper::GimpSizeToScreen(textureSize * in);
    } else {
        LOGF("Unkown vec2singlefloat modifier '" << INV_HASH(modifier) << "' (" << modifier << ')');
    }
}

enum IntervalMode {
    IntervalAsRandom,
    IntervalValue1,
    IntervalValue2,
};

template <class T>
int  load(const DataFileParser& dfp, hash_t section, hash_t id, IntervalMode mode, T* out);


template <>
inline int load(const DataFileParser& dfp, hash_t section, hash_t id, IntervalMode mode, glm::vec2* out) {
    float fp[4];

    int count = dfp.get(section, id, fp, 4, false);

    if (count >= 2) {
        hash_t mod = dfp.getModifier(section, id);

        glm::vec2 parsed[] = { glm::vec2(fp[0], fp[1]), glm::vec2(fp[2], fp[3]) };
        count /= 2;

        applyVec2Modifiers(mod, parsed, count);

        if (count == 2) {
            switch (mode) {
                case IntervalAsRandom: *out = Interval<glm::vec2>(parsed[0], parsed[1]).random(); break;
                case IntervalValue1: *out = parsed[0]; break;
                case IntervalValue2: *out = parsed[1]; break;
            }
        } else {
            *out = parsed[0];
        }
        return 1;
    }

    if (count == 1) {
        hash_t mod = dfp.getModifier(section, id);

        if (mod == 0)
            return 0;

        std::string textureName;
        if (dfp.get(HASH("Rendering", 0xe6cc1e11), HASH("texture", 0x3d4e3ff8), &textureName, 1, false)) {
            const glm::vec2& s = theRenderingSystem.getTextureSize(textureName.c_str());
            applyVec2SingleFloatModifiers(mod, s, fp[0], out);
            return 1;
        }
    }

    return 0;
}

template <>
inline int load(const DataFileParser& dfp, hash_t section, hash_t id, IntervalMode mode, Color* out) {
    {
        float p[8];

        int count = dfp.get(section, id, p, 8, false);

        switch (count) {
            case 8: {
                Interval<Color> itv(Color(&p[0], 0xffffffff), Color(&p[4], 0xffffffff));
                switch (mode) {
                    case IntervalAsRandom: *out = itv.random(); break;
                    case IntervalValue1: *out = itv.t1; break;
                    case IntervalValue2: *out = itv.t2; break;
                }
                return 1;
            }
            case 4:
            case 3: {
                *out = Color(&p[0], 0xffffffff);
                if (count == 3)
                    out->a = 1;
                return 1;
            }
        }
    }

    {
        std::string html;
        if (dfp.get(section, id, &html, 1, false)) {
            hash_t modifier = dfp.getModifier(section, id);

            if (modifier == HASH("html", 0xa7a6d925)) {
                int32_t h;
                std::istringstream iss(html);
                iss >> std::hex >> h;
                *out = Color(((h >> 16) & 0xff) / 255.0f
                    , ((h >> 8) & 0xff) / 255.0f
                    , ((h >> 0) & 0xff) / 255.0f
                    , 1.f);
                return 1;
            } else if (modifier == HASH("name", 0x195267c7)) {
                *out = Color(html);

                return 1;
            }
        }
    }
    return 0;
}

template <>
inline int load(const DataFileParser& dfp, hash_t section, hash_t id, IntervalMode, std::string* out) {
    std::string parsed;

    // %loc handled by caller
    if (dfp.get(section, id, &parsed, 1, false)) {
        // we got a single value
        *out = parsed;

        return 1;
    } else {
        // fail
        return 0;
    }
}

template <>
inline int load(const DataFileParser& dfp, hash_t section, hash_t id, IntervalMode mode, float* out) {
    float parsed[2];

    const int count = dfp.get(section, id, parsed, 2, false);

    if (count > 0) {
        applyFloatModifiers(dfp.getModifier(section, id), parsed, count);

        if (count == 2) {
            // we got an interval
            Interval<float> itv(parsed[0], parsed[1]);
            switch (mode) {
                case IntervalAsRandom: *out = itv.random(); break;
                case IntervalValue1: *out = itv.t1; break;
                case IntervalValue2: *out = itv.t2; break;
            }
        } else {
            *out = parsed[0];
        }

        return 1;
    }
    return 0;
}

template <class T>
inline int  load(const DataFileParser& dfp, hash_t section, hash_t id, IntervalMode mode, T* out) {
    T parsed[2];

    int count = dfp.get(section, id, parsed, 2, false);

    if (count == 2) {
        // we got an interval
        Interval<T> itv(parsed[0], parsed[1]);
        switch (mode) {
            case IntervalAsRandom: *out = itv.random(); break;
            case IntervalValue1: *out = itv.t1; break;
            case IntervalValue2: *out = itv.t2; break;
        }

        return 1;
    } else if (mode == IntervalAsRandom && count == 1) {
        // we got a single value
        *out = parsed[0];

        return 1;
    } else {
        // fail
        return 0;
    }
}

static bool loadSingleProperty(const std::string& context,
        const DataFileParser& dfp,
        hash_t section,
        hash_t id,
        PropertyType::Enum type,
        PropertyAttribute::Enum attr,
        PropertyNameValueMap& propMap,
        EntityTemplate& templ);

int ComponentFactory::build(
        const std::string& context,
        const DataFileParser& dfp,
        hash_t section,
        const std::vector<IProperty*>& properties, EntityTemplate& templ) {
    LOGT_EVERY_N(30, "Add \%copy modifier, to copy the value of another entity property");

    // lookup ComponentSystem by its name
    ComponentSystem* systm = ComponentSystem::GetById(section);
    LOGE_IF(!systm, "Missing system: '" << INV_HASH(section) << "'");
    if (!systm)
        return 0;

    PropertyNameValueMap& propMap = templ.properties[systm];
    // Cleanup current values
    for (auto it : propMap) {
        delete it.second;
    }

#if SAC_DEBUG
    const auto propertiesInFile = dfp.sectionSize(section);
    std::list<hash_t> loaded;
#endif

    LOGV(2, "Build system: " << INV_HASH(section));

    // Browse properties for the given system
    for (auto it = properties.begin(); it!=properties.end(); ++it) {
        // Retrieve property name
        auto* prop = *it;
        const hash_t id = prop->getId();
        const auto type = prop->getType();

        // Try to load property from data
        bool success = loadSingleProperty(context, dfp, section, id, type, prop->getAttribute(), propMap, templ);

        // special testing case
        if (!success && id == HASH("position", 0xffab91ef)) {
            for (unsigned i=0; i<8; i++) {
                if (loadSingleProperty(context, dfp, section, positionHack[i], type, prop->getAttribute(), propMap, templ)) {
                    success = true;
                    break;
                }
            }
        }
#if SAC_DEBUG
        if (success) {
            loaded.push_back(id);
        }
#endif
        LOGV(3, "  * " << INV_HASH(id) << std::dec << ": " << (success ? "loaded":"missing"));
    }

#if SAC_DEBUG
    if (loaded.size() != propertiesInFile) {
        LOGE(propertiesInFile << " declared in " << context << ".entity [" << INV_HASH(section) << "] and only " << loaded.size() << " actually loaded");
        LOGV(1, "Loaded:");
        for (auto& s: loaded) {
            LOGV(1, "   '0x" << std::hex << s << "'" << std::dec);
        }
        LOGV(1, "Missing:");
        for (unsigned i=0; i<propertiesInFile; ++i) {
            std::string key, value;
            dfp.get(section, i, key, &value);

            hash_t h = Murmur::RuntimeHash(key.c_str());
            if (std::find(loaded.begin(), loaded.end(), h) == loaded.end()) {
                LOGE("   '" << key << "' 0x" << std::hex << h << std::dec << " = ... not loaded");
            }
        }
    }
#endif
    return propMap.size();
}

void ComponentFactory::applyTemplate(Entity entity, void* component, const PropertyNameValueMap& propValueMap, const std::vector<IProperty*>& properties, LocalizeAPI* localizeAPI) {
    #define TYPE_2_PTR(_type_) (_type_ * )((uint8_t*)component + prop->offset)
    #define ASSIGN2(_type1_, _type2_) { \
        Interval< _type1_ > itv; \
        memcpy(&itv, (*it).second, sizeof(itv)); \
        if (prop->getAttribute() == PropertyAttribute::Interval){ \
            (TYPE_2_PTR(Interval < _type2_ > ))->t1 = itv.t1; \
            (TYPE_2_PTR(Interval < _type2_ > ))->t2 = itv.t2; \
        } else \
            *(TYPE_2_PTR(_type2_)) = itv.random(); }

    #define ASSIGN(_type_) { \
        Interval< _type_ > itv; \
        memcpy(&itv, (*it).second, sizeof(itv)); \
        if (prop->getAttribute() == PropertyAttribute::Interval) \
            *(TYPE_2_PTR(Interval < _type_ > )) = itv; \
        else \
            *(TYPE_2_PTR(_type_)) = itv.random(); }

    int positionHackIndex = -1;
    for (IProperty* prop : properties) {
        hash_t id = prop->getId();
        auto it = propValueMap.find(id);
        if (it == propValueMap.end()) {
            // special testing case
            if (id == HASH("position", 0xffab91ef)) {
                for (unsigned i=0; i<8; i++) {
                    it = propValueMap.find(positionHack[i]);
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
            case PropertyType::Int8:
                ASSIGN2(int, int8_t);
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
                    VectorProperty<std::string> vp(HASH("dummy", 0x89776688), 0);
                    vp.deserialize((*it).second, out);
                } else {
                    unsigned l;
                    memcpy(&l, (*it).second, sizeof(int));
                    bool toLocalize;
                    memcpy(&toLocalize, (*it).second + sizeof(int), sizeof(bool));
                    char* tmp = (char*)alloca(l);
                    memcpy(tmp, (*it).second + sizeof(int) + sizeof(bool), l);
                    tmp[l] = '\0';
                    std::string* s = TYPE_2_PTR(std::string);
                    if (toLocalize)
                        *s = localizeAPI->text(tmp);
                    else
                        *s = std::string(tmp, l);
                }
                break;
            }
            case PropertyType::Color:
                ASSIGN(Color);
                break;
            case PropertyType::Sound:
            case PropertyType::Texture:
            case PropertyType::Hash:
                memcpy((uint8_t*)component + prop->offset, (*it).second, sizeof(TextureRef));
                break;
            case PropertyType::Entity: {
                #if 0
                uint8_t* a = (*it).second;
                if (a[0] == 0) {
                    LOGT("sub entities");

                    a++;
                    EntityTemplateRef r;
                    memcpy(&r, a, sizeof(r));
                    Entity* e = TYPE_2_PTR(Entity);
                    if (*e) {
                        theEntityManager.DeleteEntity(*e);
                    }
                    *e = theEntityManager.CreateEntity("sub_" + prop->getName(), EntityType::Volatile, r);
                    ANCHOR(*e)->parent = entity;
                } else if (a[0] == 1)
                #endif
                {
                    hash_t h;
                    memcpy(&h, (*it).second, sizeof(hash_t));
                    Entity byName = theEntityManager.getEntityByName(h);
                    LOGF_IF(byName <= 0, "Invalid entity requested by name: " << h
                    #if SAC_DEBUG
                    << " (" << Murmur::lookup(h) << ')'
                    #endif
                    << " for property: '" << id << "'"
                    #if SAC_DEBUG
                    << " (" << Murmur::lookup(id) << ')'
                    #endif
                    );
                    memcpy(TYPE_2_PTR(Entity), &byName, sizeof(Entity));
                }
                break;
            }
            default:
                break;
        }
    }

    if (positionHackIndex >= 0) {
        const glm::vec2 coeff[] = {
            glm::vec2(-0.5, 0.5) , glm::vec2(0, 0.5) , glm::vec2(0.5, 0.5),
            glm::vec2(-0.5, 0.0)                     , glm::vec2(0.5, 0.0),
            glm::vec2(-0.5, -0.5), glm::vec2(0, -0.5), glm::vec2(0.5, -0.5),
        };
        // find position
        for (IProperty* prop : properties) {
            if (prop->getId() == HASH("position", 0xffab91ef)) {
                glm::vec2* position = TYPE_2_PTR(glm::vec2);
                *position =
                    AnchorSystem::adjustPositionWithAnchor(*position, TRANSFORM(entity)->size * coeff[positionHackIndex]);
                break;
            }
        }
    }
}

static bool loadSingleProperty(const std::string&,
        const DataFileParser& dfp,
        hash_t section,
        hash_t id,
        PropertyType::Enum type,
        PropertyAttribute::Enum attr,
        PropertyNameValueMap& propMap,
        EntityTemplate& templ) {

    #define LOAD_INTERVAL_TEMPL(_type_) { \
        Interval<_type_> itv; \
        bool success = load(dfp, section, id, IntervalValue1, &itv.t1); \
        if(success) load(dfp, section, id, IntervalValue2, &itv.t2);\
        else { success = load(dfp, section, id, IntervalAsRandom, &itv.t1); itv.t2 = itv.t1; } \
        if (success) {\
        LOGV(1, "Loaded " << INV_HASH(section) << "/" << INV_HASH(id) << " property. Value=[" << itv.t1 << ", " << itv.t2 << ']'); \
        uint8_t* arr = new uint8_t[sizeof(itv)];\
        memcpy(arr, &itv, sizeof(itv));\
        propMap.insert(std::make_pair(id, arr)); return true; }}

    // temp buffer
    char* temp = (char*)alloca(512);

    switch (type) {
        case PropertyType::Float:
            LOAD_INTERVAL_TEMPL(float);
            break;
        case PropertyType::Int:
        case PropertyType::Int8:
            LOAD_INTERVAL_TEMPL(int);
            break;
        case PropertyType::Bool:
            LOAD_INTERVAL_TEMPL(bool);
            break;
        case PropertyType::Vec2:
            LOAD_INTERVAL_TEMPL(glm::vec2);
            break;
        case PropertyType::String: {
            // let's try something simple here
            if (attr == PropertyAttribute::Vector) {
                int splits = dfp.getSubStringCount(section, id);
                if (splits > 0) {
                    std::string* all = new std::string[splits];
                    if (dfp.get(section, id, all, splits, true)) {
                        std::vector<std::string> a;
                        for (int i=0; i<splits; i++)
                            a.push_back(all[i]);

                        VectorProperty<std::string> vp(HASH("dummy", 0x89776688), 0);
                        unsigned size = vp.size(&a);
                        uint8_t* arr = new uint8_t[size];
                        vp.serialize(arr, &a);
                        propMap.insert(std::make_pair(id, arr));
                        delete[] all;
                        return true;
                    }
                    delete[] all;
                }
            } else {
                int len = 0;
                if ((len = dfp.get(section, id, temp, 512, false))) {
                    bool toLocalize = (dfp.getModifier(section, id) == HASH("loc", 0x3fbcd7cb));

                    uint8_t* arr = new uint8_t[sizeof(int) + sizeof(bool) + len];
                    memcpy(arr, &len, sizeof(int));
                    memcpy(&arr[sizeof(int)], &toLocalize, sizeof(bool));
                    memcpy(&arr[sizeof(int) + sizeof(bool)], temp, len);
                    propMap.insert(std::make_pair(id, arr));

                    LOGV(1, "Loaded " << INV_HASH(section) << "/" << INV_HASH(id) << " property. Value='" << temp << "' (localisable: " << toLocalize << ')');
                    return true;
                }
            }
            break;
        }
        case PropertyType::Entity: {
            if (dfp.get(section, id, temp, 512, false)) {
                if (dfp.getModifier(section, id) == HASH("name", 0x195267c7)) {
                    uint8_t* arr = new uint8_t[sizeof(hash_t)];
                    hash_t h = Murmur::RuntimeHash(temp);
                    memcpy(arr, &h, sizeof(hash_t));
                    propMap.insert(std::make_pair(id, arr));
                    templ.dependencies.push_back(h);
                    return true;
                }
            }
            break;
        }
        case PropertyType::Color:
            LOAD_INTERVAL_TEMPL(Color);
            break;
        case PropertyType::Sound: {
            if (dfp.get(section, id, temp, 512, false)) {
                uint8_t* arr = new uint8_t[sizeof(TextureRef)];
                *((SoundRef*)arr) = theSoundSystem.loadSoundFile(temp);
                propMap.insert(std::make_pair(id, arr));
                return true;
            }
            break;
        }
        case PropertyType::Hash:
        case PropertyType::Texture: {
            if (dfp.get(section, id, temp, 512, false)) {
                uint8_t* arr = new uint8_t[sizeof(hash_t)];
                hash_t h = Murmur::RuntimeHash(temp);
                *((hash_t*)arr) = h;
                propMap.insert(std::make_pair(id, arr));
                return true;
            }
            break;
        }
        default:
            LOGW("Property '" << INV_HASH(section) << '/' << INV_HASH(id) << "' uses unhandled type " << type);
            break;
    }
    #undef LOAD_INTERVAL_TEMPL
    return false;
}
