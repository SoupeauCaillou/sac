#pragma once
#include <vector>
#include <map>
#include <cstring>
#include <stdint.h>
#include <glm/glm.hpp>
#include "base/Interval.h"
#include <string>

namespace PropertyType {
    enum Enum {
        String,
        Vec2,
        Int,
        Float,
        Color,
        Texture,
        Entity,
        Unsupported
    };
}

class IProperty {
    protected:
        IProperty(const std::string& name, PropertyType::Enum type, bool isInterval, unsigned long offset, unsigned size);
    public:
        virtual ~IProperty() {}
        virtual unsigned size(void* object) const;
        virtual bool different(void* object, void* refObject) const;
        virtual int serialize(uint8_t* out, void* object) const;
        virtual int deserialize(uint8_t* in, void* object) const;

        const std::string& getName() const { return name; }
        PropertyType::Enum getType() const {return type;}
        bool isInterval() const { return interval; }
    public:
        unsigned long offset;
        unsigned _size;
    private:
        std::string name;
        PropertyType::Enum type;
        bool interval;

};

template<typename T>
class Property : public IProperty {
    public:
        Property(const std::string& name, PropertyType::Enum type, unsigned long offset, T pEpsilon = 0);
        Property(const std::string& name, unsigned long offset, T pEpsilon = 0);
        bool different(void* object, void* refObject) const;
    private:
        T epsilon;
};

class EntityProperty : public IProperty {
    public:
        EntityProperty(const std::string& name, unsigned long offset);
        unsigned size(void* object) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(uint8_t* in, void* object) const;
};

class StringProperty : public IProperty {
    public:
        StringProperty(const std::string& name, unsigned long offset);
        unsigned size(void* object) const;
        bool different(void* object, void* refObject) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(uint8_t* in, void* object) const;
};

template <typename T>
class VectorProperty : public IProperty {
    public:
        VectorProperty(const std::string& name, unsigned long offset);
        unsigned size(void* object) const;
        bool different(void* object, void* refObject) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(uint8_t* in, void* object) const;
};

template <typename T>
class IntervalProperty : public IProperty {
    public:
        IntervalProperty(const std::string& name, unsigned long offset);
        bool different(void* object, void* refObject) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(uint8_t* in, void* object) const;
};


template <typename T, typename U>
class MapProperty : public IProperty {
    public:
        MapProperty(const std::string& name, unsigned long offset);
        virtual unsigned size(void* object) const;
        bool different(void* object, void* refObject) const;
        virtual int serialize(uint8_t* out, void* object) const;
        virtual int deserialize(uint8_t* in, void* object) const;
};

#define PTR_OFFSET_2_PTR(ptr, offset) ((uint8_t*)ptr + offset)

#define OFFSET(member, p) ((uint8_t*)&p.member - (uint8_t*)&p)

#include "Serializer.hpp"

class Serializer {
    std::vector<IProperty*> properties;

    public:
    ~Serializer();
    void add(IProperty* p) { properties.push_back(p); }
    int size(void* object);
    int serializeObject(uint8_t* out, void* object, void* refObject = 0);
    int serializeObject(uint8_t** out, void* object, void* refObject = 0);
    int deserializeObject(uint8_t* in, int size, void* object);

    const std::vector<IProperty*>& getProperties() const { return properties; }
};
