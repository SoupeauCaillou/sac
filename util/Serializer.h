/*!
 * \file Serializer.h
 * \brief 
 * \author Pierre-Eric Pelloux-Prayer
 * \author Gautier Pelloux-Prayer
 */
#pragma once
#include <vector>
#include <map>
#include <cstring>
#include <stdint.h>
#include "base/MathUtil.h"
#include "base/Interval.h"

/*!
 * \class IProperty
 * \brief
 */
class IProperty {
    protected:
        IProperty(unsigned long offset, unsigned size);
    public:
        virtual ~IProperty() {}
        virtual unsigned size(void* object) const;
        virtual bool different(void* object, void* refObject) const;
        virtual int serialize(uint8_t* out, void* object) const;
        virtual int deserialize(uint8_t* in, void* object) const;
    public:
        unsigned long offset;
        unsigned _size;
};

/*!
 * \class Property
 * \brief
 */
template<typename T>
class Property : public IProperty {
    public:
        Property(unsigned long offset, T pEpsilon = 0) : IProperty(offset, sizeof(T)), epsilon(pEpsilon) {}
        bool different(void* object, void* refObject) const;
    private:
        T epsilon;
};

/*!
 * \class EntityProperty
 * \brief
 */
class EntityProperty : public IProperty {
    public:
        EntityProperty(unsigned long offset);
        unsigned size(void* object) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(uint8_t* in, void* object) const;
};

class StringProperty : public IProperty {
    public:
        StringProperty(unsigned long offset);
        unsigned size(void* object) const;
        bool different(void* object, void* refObject) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(uint8_t* in, void* object) const;
};

/*!
 * \class VectorProperty
 * \brief
 */
template <typename T>
class VectorProperty : public IProperty {
    public:
        VectorProperty(unsigned long offset);
        unsigned size(void* object) const;
        bool different(void* object, void* refObject) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(uint8_t* in, void* object) const;
};

/*!
 * \class IntervalProperty
 * \brief
 */
template <typename T>
class IntervalProperty : public IProperty {
    public:
        IntervalProperty(unsigned long offset);
        bool different(void* object, void* refObject) const;
        int serialize(uint8_t* out, void* object) const;
        int deserialize(uint8_t* in, void* object) const;
};

/*!
 * \class MapProperty
 * \brief
 */
template <typename T, typename U>
class MapProperty : public IProperty {
    public:
        MapProperty(unsigned long offset);
        virtual unsigned size(void* object) const;
        bool different(void* object, void* refObject) const;
        virtual int serialize(uint8_t* out, void* object) const;
        virtual int deserialize(uint8_t* in, void* object) const;
};

#define PTR_OFFSET_2_PTR(ptr, offset) ((uint8_t*)ptr + offset)

#include "Serializer.hpp"

#define OFFSET(member, p) ((uint8_t*)&p.member - (uint8_t*)&p)

/*!
 * \class Serializer
 * \brief
 */
class Serializer {
    std::vector<IProperty*> properties;

    public:
    ~Serializer();
    void add(IProperty* p) { properties.push_back(p); }
    int size(void* object);
    int serializeObject(uint8_t* out, void* object, void* refObject = 0);
    int serializeObject(uint8_t** out, void* object, void* refObject = 0);
    int deserializeObject(uint8_t* in, int size, void* object);
};
