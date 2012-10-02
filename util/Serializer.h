/*
 This file is part of Heriswap.

 @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
 @author Soupe au Caillou - Gautier Pelloux-Prayer

 Heriswap is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, version 3.

 Heriswap is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once
#include <vector>
#include <cstring>
#include <stdint.h>
#include "base/MathUtil.h"
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
        VectorProperty(unsigned long offset) : Property(offset, 0) {}
        unsigned size(void* object) const {
            std::vector<T>* v = (std::vector<T>*) ((uint8_t*)object + offset);
            return sizeof(unsigned) + v->size() * sizeof(T);
        }
        bool different(void* object, void* refObject) const {
            std::vector<T>* v = (std::vector<T>*) ((uint8_t*)object + offset);
            std::vector<T>* w = (std::vector<T>*) ((uint8_t*)refObject + offset);
            if (v->size() != w->size())
                return true;
            for (unsigned i=0; i<v->size(); i++) {
                if ((*v)[i] != (*w)[i])
                    return true;
            }
            return false;
        }
        int serialize(uint8_t* out, void* object) const {
            std::vector<T>* v = (std::vector<T>*) ((uint8_t*)object + offset);
            unsigned size = v->size();
            int idx = 0;
            memcpy(out, &size, sizeof(unsigned));
            idx += sizeof(unsigned);
            for (unsigned i=0; i<size; i++) {
                memcpy(&out[idx], &(*v)[i], sizeof(T));
                idx += sizeof(T);
            }
            return idx;
        }
        int deserialize(uint8_t* in, void* object) const {
            std::vector<T>* v = (std::vector<T>*) ((uint8_t*)object + offset);
            v->clear();
            unsigned size;
            memcpy(&size, in, sizeof(unsigned));
            int idx = sizeof(unsigned);
            v->reserve(size);
            for (unsigned i=0; i<size; i++) {
                T t;
                memcpy(&t, &in[idx], sizeof(T));
                v->push_back(t);
                idx += sizeof(T);
            }
            return idx;
        }
};

#define OFFSET(member, p) ((uint8_t*)&p.member - (uint8_t*)&p)

class Serializer {
    std::vector<Property*> properties;
    
    public:
    void add(Property* p) { properties.push_back(p); }
    int size(void* object);
    int serializeObject(uint8_t* out, void* object, void* refObject = 0);
    int serializeObject(uint8_t** out, void* object, void* refObject = 0);
    int deserializeObject(uint8_t* in, int size, void* object);
};
