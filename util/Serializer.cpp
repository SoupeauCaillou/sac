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



#include "SerializerProperty.h"
#include "systems/System.h"
#if SAC_NETWORK
#include "systems/NetworkSystem.h"
#endif

#include "SerializerProperty.hpp"

template class Property<bool>;
template class Property<int>;
template class Property<unsigned int>;
template class Property<unsigned short>;
template class Property<float>;
template class Property<glm::vec2>;
template class Property<Color>;
template class Property<unsigned char>;
template class Property<signed char>;
template class IntervalProperty<Color>;
template class IntervalProperty<float>;
template class VectorProperty<std::string>;
template class VectorProperty<int>;
template class MapProperty<int, float>;
template class MapProperty<std::string, float>;

Serializer::~Serializer() {
    for (unsigned i=0; i<properties.size(); i++) {
        delete properties[i];
    }
    properties.clear();
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
            unsigned propSize = properties[i]->size(object);
            s += 1 + propSize;
            LOGE_IF (s < 0, "Invalid size");
        }
    }
    if (s == 0)
        return 0;
    *out = new uint8_t[s];
    return serializeObject(*out, object, refObject);
}

int Serializer::deserializeObject(const uint8_t* in, int size, void* object) {
    int index = 0;
    while (index < size) {
        uint8_t prop = in[index++];
        index += properties[prop]->deserialize(&in[index], object);
    }
    return index;
}
