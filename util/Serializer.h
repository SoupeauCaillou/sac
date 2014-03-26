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
#include <vector>
#include <map>
#include <cstring>
#include <stdint.h>
#include <glm/glm.hpp>
#include "base/Interval.h"
#include "util/MurmurHash.h"
#include <string>

namespace PropertyType {
    enum Enum {
        String,
        Vec2,
        Int,
        Int8,
        Float,
        Color,
        Texture,
        Sound,
        Entity,
        Bool,
        Hash,
        Unsupported
    };
}

namespace PropertyAttribute {
    enum Enum {
        None,
        Interval,
        Vector
    };
}

class IProperty {
    protected:
        IProperty(hash_t id, PropertyType::Enum type, PropertyAttribute::Enum attribute, unsigned long offset, unsigned size);
    public:
        virtual ~IProperty() {}
        virtual unsigned size(void* object) const;
        virtual bool different(void* object, void* refObject) const;
        virtual int serialize(uint8_t* out, void* object) const;
        virtual int deserialize(const uint8_t* in, void* object) const;

        hash_t getId() const { return id; }
        PropertyType::Enum getType() const {return type;}
        PropertyAttribute::Enum getAttribute() const { return attribute; }
    public:
        unsigned long offset;
        unsigned _size;
    private:
        hash_t id;
        PropertyType::Enum type;
        PropertyAttribute::Enum attribute;

};

template<typename T>
class Property : public IProperty {
    public:
        Property(hash_t id, PropertyType::Enum type, unsigned long offset, T pEpsilon = 0);
        Property(hash_t id, unsigned long offset, T pEpsilon = 0);
        bool different(void* object, void* refObject) const;
    private:
        T epsilon;
};

class EntityProperty : public IProperty {
    public:
        EntityProperty(hash_t id, unsigned long offset);
        unsigned size(void* object) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(const uint8_t* in, void* object) const;
};

class StringProperty : public IProperty {
    public:
        StringProperty(hash_t id, unsigned long offset);
        unsigned size(void* object) const;
        bool different(void* object, void* refObject) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(const uint8_t* in, void* object) const;
};

template <typename T>
class VectorProperty : public IProperty {
    public:
        VectorProperty(hash_t id, unsigned long offset);
        unsigned size(void* object) const;
        bool different(void* object, void* refObject) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(const uint8_t* in, void* object) const;
};

template <typename T>
class IntervalProperty : public IProperty {
    public:
        IntervalProperty(hash_t id, unsigned long offset);
        bool different(void* object, void* refObject) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(const uint8_t* in, void* object) const;
};


template <typename T, typename U>
class MapProperty : public IProperty {
    public:
        MapProperty(hash_t id, unsigned long offset);
        virtual unsigned size(void* object) const;
        bool different(void* object, void* refObject) const;
        virtual int serialize(uint8_t* out, void* object) const;
        virtual int deserialize(const uint8_t* in, void* object) const;
};

#define PTR_OFFSET_2_PTR(ptr, offset) ((uint8_t*)ptr + offset)

#define OFFSET(member, p) ((uint8_t*)&p.member - (uint8_t*)&p)
#define OFFSET_PTR(member, ptr) ((uint8_t*)&ptr->member - (uint8_t*)ptr)

#include "Serializer.hpp"

class Serializer {
    std::vector<IProperty*> properties;

    public:
    ~Serializer();
    void add(IProperty* p) { properties.push_back(p); }
    int size(void* object);
    int serializeObject(uint8_t* out, void* object, void* refObject = 0);
    int serializeObject(uint8_t** out, void* object, void* refObject = 0);
    int deserializeObject(const uint8_t* in, int size, void* object);

    const std::vector<IProperty*>& getProperties() const { return properties; }
};
