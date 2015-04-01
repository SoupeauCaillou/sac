#include "util/SerializerProperty.h"
#include <cstring>
#include <sstream>
#include <iomanip>

#include "base/Entity.h"

#define PTR_OFFSET_2_PTR(ptr, offset) ((uint8_t*)ptr + offset)

IProperty::IProperty(hash_t pId, PropertyType::Enum pType, PropertyAttribute::Enum attr, unsigned long pOffset, unsigned pSize) : offset(pOffset), _size(pSize), id(pId), type(pType), attribute(attr) {

}

unsigned IProperty::size(void*) const {
    return _size;
}

bool IProperty::different(void* object, void* refObject) const {
    return (memcmp(PTR_OFFSET_2_PTR(object, offset), PTR_OFFSET_2_PTR(refObject, offset), _size) != 0);
}

int IProperty::serialize(uint8_t* out, void* object) const {
    memcpy(out, PTR_OFFSET_2_PTR(object, offset), _size);
    return _size;
}

int IProperty::deserialize(const uint8_t* in, void* object) const {
    memcpy(PTR_OFFSET_2_PTR(object, offset), in, _size);
    return _size;
}

EntityProperty::EntityProperty(hash_t id, unsigned long offset) : IProperty(id, PropertyType::Entity, PropertyAttribute::None, offset, sizeof(Entity)) {

}

unsigned EntityProperty::size(void*) const {
#if SAC_NETWORK
    return sizeof(unsigned int);
#else
    return sizeof(Entity);
#endif
}

int EntityProperty::serialize(uint8_t* out, void* object) const {
    Entity* e = (Entity*) PTR_OFFSET_2_PTR(object, offset);
#if SAC_NETWORK
    if (NetworkSystem::GetInstancePointer()) {
        unsigned int guid = theNetworkSystem.entityToGuid(*e);
        memcpy(out, &guid, sizeof(guid));
        return sizeof(unsigned int);
    }
#endif
    memcpy(out, e, sizeof(Entity));
    return sizeof(Entity);
}

int EntityProperty::deserialize(const uint8_t* in, void* object) const {
#if SAC_NETWORK
    if (NetworkSystem::GetInstancePointer()) {
        unsigned int guid;
        memcpy(&guid, in, sizeof(guid));
        Entity e = theNetworkSystem.guidToEntity(guid);
        memcpy(PTR_OFFSET_2_PTR(object, offset), &e, sizeof(Entity));
        return sizeof(unsigned int);
    }
#endif
    memcpy(PTR_OFFSET_2_PTR(object, offset), in, sizeof(Entity));
    return sizeof(Entity);
}

StringProperty::StringProperty(hash_t id, unsigned long pOffset) : IProperty(id, PropertyType::String, PropertyAttribute::None, pOffset, 0) {}

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

int StringProperty::deserialize(const uint8_t* in, void* object) const {
    uint8_t length = in[0];
    std::string* a = (std::string*) PTR_OFFSET_2_PTR(object, offset);
    *a = std::string((const char*)&in[1], length);
    return length + 1;
}