#include <algorithm>
#include <string>
#include <sstream>
#include "base/Color.h"

template <>
inline bool Property<Color>::different(void* object, void* refObject) const {
    Color* a = (Color*) ((uint8_t*)object + offset);
    Color* b = (Color*) ((uint8_t*)refObject + offset);
    return *a != *b;
}

template <>
inline bool Property<Vector2>::different(void* object, void* refObject) const {
    Vector2* a = (Vector2*) ((uint8_t*)object + offset);
    Vector2* b = (Vector2*) ((uint8_t*)refObject + offset);
    return (*a - *b).LengthSquared() > epsilon.LengthSquared();
}

template <typename T>
inline bool Property<T>::different(void* object, void* refObject) const {
    T* a = (T*) ((uint8_t*)object + offset);
    T* b = (T*) ((uint8_t*)refObject + offset);
    return (MathUtil::Abs(*a - *b) > epsilon);
}

template <typename T>
VectorProperty<T>::VectorProperty(unsigned long offset) : IProperty(offset, 0) {}

template <typename T>
inline unsigned VectorProperty<T>::size(void* object) const {
    std::vector<T>* v = (std::vector<T>*) ((uint8_t*)object + offset);
    return sizeof(unsigned) + v->size() * sizeof(T);
}

template <typename T>
inline bool VectorProperty<T>::different(void* object, void* refObject) const {
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

template <typename T>
inline int VectorProperty<T>::serialize(uint8_t* out, void* object) const {
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

template <typename T>
inline int VectorProperty<T>::deserialize(uint8_t* in, void* object) const {
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

template <typename T>
IntervalProperty<T>::IntervalProperty(unsigned long offset) : IProperty(offset, 2 * sizeof(T)) {}

template <typename T>
inline bool IntervalProperty<T>::different(void* object, void* refObject) const {
    Interval<T>* v = (Interval<T>*) ((uint8_t*)object + offset);
    Interval<T>* w = (Interval<T>*) ((uint8_t*)refObject + offset);
    return v->t1 != w->t1 || v->t2 != w->t2;
}

template <typename T>
inline int IntervalProperty<T>::serialize(uint8_t* out, void* object) const {
    Interval<T>* v = (Interval<T>*) ((uint8_t*)object + offset);
    memcpy(out, &v->t1, sizeof(T));
    memcpy(out + sizeof(T), &v->t2, sizeof(T));
    return 2 * sizeof(T);
}

template <typename T>
inline int IntervalProperty<T>::deserialize(uint8_t* in, void* object) const {
    Interval<T>* v = (Interval<T>*) ((uint8_t*)object + offset);
    memcpy(&v->t1, in, sizeof(T));
    memcpy(&v->t2, in + sizeof(T), sizeof(T));
    return 2 * sizeof(T);
}

template <typename T, typename U>
MapProperty<T,U>::MapProperty(unsigned long offset) : IProperty(offset, 0) {}

template <typename T, typename U>
inline unsigned MapProperty<T,U>::size(void* object) const {
    std::map<T, U>* v = (std::map<T, U>*) ((uint8_t*)object + offset);
    return sizeof(unsigned) + v->size() * (sizeof(T) + sizeof(U));
}

template <typename T, typename U>
inline bool MapProperty<T,U>::different(void* object, void* refObject) const {
    std::map<T, U>* v = (std::map<T, U>*) ((uint8_t*)object + offset);
    std::map<T, U>* w = (std::map<T, U>*) ((uint8_t*)refObject + offset);
    if (v->size() != w->size())
        return true;
    for (typename std::map<T, U>::iterator it=v->begin(), jt=w->begin(); it!=v->end(); ++it, ++jt) {
        if (v->find(jt->first) == v->end())
            return true;
        if (w->find(it->first) == w->end())
            return true;
    }
    return false;
}

template <typename T, typename U>
inline int MapProperty<T,U>::serialize(uint8_t* out, void* object) const {
    std::map<T, U>* v = (std::map<T, U>*) ((uint8_t*)object + offset);
    unsigned size = v->size();
    int idx = 0;
    memcpy(out, &size, sizeof(unsigned));
    idx += sizeof(unsigned);
    for (typename std::map<T, U>::iterator it=v->begin(); it!=v->end(); ++it) {
        memcpy(&out[idx], &(it->first), sizeof(T));
        idx += sizeof(T);
        memcpy(&out[idx], &(it->second), sizeof(U));
        idx += sizeof(U);
    }
    return idx;
}

template <typename T, typename U>
inline int MapProperty<T,U>::deserialize(uint8_t* in, void* object) const {
    std::map<T, U>* v = (std::map<T, U>*) ((uint8_t*)object + offset);
    v->clear();
    unsigned size;
    memcpy(&size, in, sizeof(unsigned));
    int idx = sizeof(unsigned);
    for (unsigned i=0; i<size; i++) {
        T t;
        memcpy(&t, &in[idx], sizeof(T));
        idx += sizeof(T);
        U u;
        memcpy(&u, &in[idx], sizeof(T));
        idx += sizeof(U);
        v->insert(std::make_pair(t, u));
    }
    return idx;
}

template <>
inline unsigned MapProperty<std::string, float>::size(void* object) const {
    std::map<std::string, float>* v = (std::map<std::string, float>*) ((uint8_t*)object + offset);
    int size = sizeof(unsigned);
    for (std::map<std::string, float>::iterator it=v->begin(); it!=v->end(); ++it) {
        size += 1 + it->first.size() + sizeof(float);
    }
    return size;
}


template <>
inline int MapProperty<std::string, float>::serialize(uint8_t* out, void* object) const {
    std::map<std::string, float>* v = (std::map<std::string, float>*) ((uint8_t*)object + offset);
    unsigned size = v->size();
    int idx = 0;
    memcpy(out, &size, sizeof(unsigned));
    idx += sizeof(unsigned);
    for ( std::map<std::string, float>::iterator it=v->begin(); it!=v->end(); ++it) {
        uint8_t length = (uint8_t)it->first.length();
        memcpy(&out[idx++], &length, 1);
        memcpy(&out[idx], it->first.c_str(), length);
        idx += length;
        memcpy(&out[idx], &(it->second), sizeof(float));
        idx += sizeof(float);
    }
    return idx;
}

template <>
inline int MapProperty<std::string, float>::deserialize(uint8_t* in, void* object) const {
    std::map<std::string, float>* v = (std::map<std::string, float>*) ((uint8_t*)object + offset);
    v->clear();
    unsigned size;
    char tmp[256];
    memcpy(&size, in, sizeof(unsigned));
    int idx = sizeof(unsigned);
    for (unsigned i=0; i<size; i++) {
        uint8_t length;
        memcpy(&length, &in[idx++], 1);
        memcpy(tmp, &in[idx], length);
        tmp[length] = '\0';
        idx += length;
        float f;
        memcpy(&f, &in[idx], sizeof(float));
        idx += sizeof(float);
        v->insert(std::make_pair(std::string(tmp), f));
    }
    return idx;
}


