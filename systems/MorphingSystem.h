/*!
 * \file MorphingSystem.h
 * \brief 
 * \author Pierre-Eric Pelloux-Prayer
 * \author Gautier Pelloux-Prayer
 */
#pragma once

#include "System.h"

/*! \struct MorphElement
 *  \brief */
struct MorphElement {
	MorphElement() : ended(false), coeff(1) {};
	virtual ~MorphElement() {}

    /*! \brief
     *  \param t */
    virtual void lerp(float t)=0;

    /*! \brief */
    virtual void reverse() = 0;
    bool ended; //!<
    float coeff; //!<
};

/*! \struct TypedMorphElement
 *  \brief */
template<typename T>
struct TypedMorphElement : public MorphElement{
    TypedMorphElement(T* _output, const T& _from, const T& _to) : output(_output), from(_from), to(_to) {}

    T* output; //!<
    T from, to; //!<

    /*! \brief
     *  \param t */
    void lerp(float t) {
        *output = to * t + from * (1-t);
    }

    /*! \brief */
    void reverse() {
	    T c = to;
	    to = from;
	    from = c;
    }
};

/*! \struct MorphingComponent
 *  \brief */
struct MorphingComponent {
	MorphingComponent() : active(false), value(0), activationTime(0), timing(0) {}
	bool active; //!< 

	std::vector<MorphElement*> elements; //!<

    float value; //!<
	float activationTime; //!<
	float timing; //!<
};

#define theMorphingSystem MorphingSystem::GetInstance()
#define MORPHING(entity) theMorphingSystem.Get(entity)

/*! \class Morphing
 *  \brief */
UPDATABLE_SYSTEM(Morphing)

public:
    /*! \brief
     *  \param mc */
	void reverse(MorphingComponent* mc);

    /*! \brief
     *  \param mc */
	void clear(MorphingComponent* mc);
};
