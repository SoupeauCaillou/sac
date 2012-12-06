#pragma once
#include <vector>
#include <map>
#include <cstring>
#include <stdint.h>
#include "base/MathUtil.h"
#include "base/Interval.h"

class Property {
    public:
        Property(unsigned long offset, unsigned size);
        virtual unsigned size(void* object) const;
        virtual bool different(void* object, void* refObject) const;
        virtual int serialize(uint8_t* out, void* object) const;
        virtual int deserialize(uint8_t* in, void* object) const;
    protected:
        unsigned long offset;
        unsigned _size;
};

class EntityProperty : public Property {
    public:
        EntityProperty(unsigned long offset);
        unsigned size(void* object) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(uint8_t* in, void* object) const;
};

template <typename T>
class EpsilonProperty : public Property {
    public:
        EpsilonProperty(unsigned long offset, T pEpsilon) : Property(offset, sizeof(T)), epsilon(pEpsilon) {}
            bool different(void* object, void* refObject) const {
            T* a = (T*) ((uint8_t*)object + offset);
            T* b = (T*) ((uint8_t*)refObject + offset);
            return (MathUtil::Abs(*a - *b) > epsilon);
        }
    private:
        T epsilon;
};

class StringProperty : public Property {
    public:
        StringProperty(unsigned long offset);
        unsigned size(void* object) const;
        bool different(void* object, void* refObject) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(uint8_t* in, void* object) const;
};

template <typename T>
class VectorProperty : public Property {
    public:
        VectorProperty(unsigned long offset);
        unsigned size(void* object) const;
        bool different(void* object, void* refObject) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(uint8_t* in, void* object) const;
};

template <typename T>
class IntervalProperty : public Property {
    public:
        IntervalProperty(unsigned long offset);
        bool different(void* object, void* refObject) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(uint8_t* in, void* object) const;
};


template <typename T, typename U>
class MapProperty : public Property {
    public:
        MapProperty(unsigned long offset);
        virtual unsigned size(void* object) const;
        bool different(void* object, void* refObject) const;
        virtual int serialize(uint8_t* out, void* object) const;
        virtual int deserialize(uint8_t* in, void* object) const;
};

#include "Serializer.hpp"

#define OFFSET(member, p) ((uint8_t*)&p.member - (uint8_t*)&p)

class Serializer {
    std::vector<Property*> properties;

    public:
    ~Serializer();
    void add(Property* p) { properties.push_back(p); }
    int size(void* object);
    int serializeObject(uint8_t* out, void* object, void* refObject = 0);
    int serializeObject(uint8_t** out, void* object, void* refObject = 0);
    int deserializeObject(uint8_t* in, int size, void* object);
};
