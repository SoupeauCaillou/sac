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
#include <stdint.h>

class IProperty;

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
