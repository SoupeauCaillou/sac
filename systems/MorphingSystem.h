#pragma once

#include "base/MathUtil.h"

#include "System.h"

struct MorphElement {
	MorphElement() : ended(false) {};
	virtual ~MorphElement() {}
    virtual void lerp(float t)=0;
    virtual void reverse() = 0;
    bool ended;
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
#define MORPHING(entity) theMorphingSystem.Get(entity)

UPDATABLE_SYSTEM(Morphing)

public:
	void reverse(MorphingComponent* mc);
};
