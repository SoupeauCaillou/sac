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

#if !DISABLE_MORPHING_SYSTEM
#include "System.h"

struct MorphElement {
    MorphElement() : ended(false), coeff(1) {};
    virtual ~MorphElement() {}
    virtual void lerp(float t)=0;
    virtual void reverse() = 0;
    bool ended;
    float coeff;
};

template<typename T>
struct TypedMorphElement : public MorphElement{
    TypedMorphElement(T* _output, const T& _from, const T& _to) : output(_output), from(_from), to(_to) {}

    T* output;
    T from, to;
    void lerp(float t) {
        *output = to * t + from * (1-t);
    }

    void reverse() {
        T c = to;
        to = from;
        from = c;
    }
};

struct MorphingComponent {
    MorphingComponent() : active(false), value(0), activationTime(0), timing(0) {}
    bool active;

    std::vector<MorphElement*> elements;

    float value;
    float activationTime;
    float timing;
};

#define theMorphingSystem MorphingSystem::GetInstance()
#if SAC_DEBUG
#define MORPHING(entity) theMorphingSystem.Get(entity,true,__FILE__,__LINE__)
#else
#define MORPHING(entity) theMorphingSystem.Get(entity)
#endif

UPDATABLE_SYSTEM(Morphing)

public:
    void reverse(MorphingComponent* mc);
    void clear(MorphingComponent* mc);
};

#endif
