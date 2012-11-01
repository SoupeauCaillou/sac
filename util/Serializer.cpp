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
#include "Serializer.h"
#include "systems/System.h"
#ifdef SAC_NETWORK
#include "systems/NetworkSystem.h"
#endif
#include <cstring>

#define PTR_OFFSET_2_PTR(ptr, offset) ((uint8_t*)ptr + offset)

Property::Property(unsigned long pOffset, unsigned pSize) : offset(pOffset), _size(pSize) {
 
}

unsigned Property::size(void* object __attribute__((unused))) const {
    return _size;
}

bool Property::different(void* object, void* refObject) const {
    return (memcmp(PTR_OFFSET_2_PTR(object, offset), PTR_OFFSET_2_PTR(refObject, offset), _size) != 0);
}

int Property::serialize(uint8_t* out, void* object) const {
    memcpy(out, PTR_OFFSET_2_PTR(object, offset), _size);
    return _size;
}

int Property::deserialize(uint8_t* in, void* object) const {
    memcpy(PTR_OFFSET_2_PTR(object, offset), in, _size);
    return _size;
}

#ifdef SAC_NETWORK
EntityProperty::EntityProperty(unsigned long offset) : Property(offset, sizeof(Entity)) {

}

unsigned EntityProperty::size(void* object) const {
    return sizeof(unsigned int);
}

int EntityProperty::serialize(uint8_t* out, void* object) const {
    Entity* e = (Entity*) PTR_OFFSET_2_PTR(object, offset);
    unsigned int guid = theNetworkSystem.entityToGuid(*e);
    memcpy(out, &guid, sizeof(guid));
    return sizeof(guid);
}

int EntityProperty::deserialize(uint8_t* in, void* object) const {
    unsigned int guid;
    memcpy(&guid, in, sizeof(guid));
    Entity e = theNetworkSystem.guidToEntity(guid);
    memcpy(PTR_OFFSET_2_PTR(object, offset), &e, sizeof(e));
    return sizeof(guid);
}
#endif

StringProperty::StringProperty(unsigned long pOffset) : Property(pOffset, 0) {}

unsigned StringProperty::size(void* object) const {
   std::string* a = (std::string*) PTR_OFFSET_2_PTR(object, offset); 
   return (a->size() + 1);
}

bool StringProperty::different(void* object, void* refObject) const {
    std::string* a = (std::string*) PTR_OFFSET_2_PTR(object, offset); 
    std::string* b = (std::string*) PTR_OFFSET_2_PTR(refObject, offset);
    return (a->compare(*b) != 0);
}

int StringProperty::serialize(uint8_t* out, void* object) const {
    std::string* a = (std::string*) PTR_OFFSET_2_PTR(object, offset); 
    uint8_t length = (uint8_t)a->size();
    memcpy(out, &length, 1);
    memcpy(&out[1], a->c_str(), length);
    return length + 1;
}

int StringProperty::deserialize(uint8_t* in, void* object) const {
    uint8_t length = in[0];
    std::string* a = (std::string*) PTR_OFFSET_2_PTR(object, offset); 
    *a = std::string((const char*)&in[1], length);
    return length + 1;
}

int Serializer::size(void* object) {
    int s = properties.size();
    for (unsigned i=0; i<properties.size(); i++) {
        s += properties[i]->size(object);
    }
    return s;
}

int Serializer::serializeObject(uint8_t* out, void* object, void* refObject) {
    int s = 0;
    for (unsigned i=0;i <properties.size(); i++) {
        if (refObject) {
            if (!properties[i]->different(object, refObject))
                continue;
        }
        out[s++] = (uint8_t)i;
        s += properties[i]->serialize(&out[s], object);
    }
    return s;
}

int Serializer::serializeObject(uint8_t** out, void* object, void* refObject) {
    int s = 0;
    for (unsigned i=0;i <properties.size(); i++) {
        if (!refObject || properties[i]->different(object, refObject)) {
            s += 1 + properties[i]->size(object);
        }
    }
    *out = new uint8_t[s];
    return serializeObject(*out, object, refObject);
}

int Serializer::deserializeObject(uint8_t* in, int size, void* object) {
    int index = 0;
    while (index < size) {
        uint8_t prop = in[index++];
        index += properties[prop]->deserialize(&in[index], object);
    }
    return index;
}
