/*!
 * \file PhysicsSystem.h
 * \brief 
 * \author Pierre-Eric Pelloux-Prayer
 * \author Gautier Pelloux-Prayer
 */
#pragma once

#include "System.h"

/*! \struct Force
 *  \brief Description of a force (physics way) */
struct Force {
    Vector2 vector; //!< Direction of force
    Vector2 point; //!< On which point

    Force(const Vector2& v, const Vector2& p) : vector(v), point(p) {}
    Force() {}
};

/*! \struct PhysicsComponent
 *  \brief ? */
struct PhysicsComponent {
    Vector2 linearVelocity; //!< linear velocity (m/s ?)
    float angularVelocity; //!< angular velocity (rad/s ?)
    float mass; //!< mass of object
    Vector2 gravity; //!< gravity to apply on object
    std::vector<std::pair<Force, float> > forces; //!< ?
    float momentOfInertia; //!< ?
};

#define thePhysicsSystem PhysicsSystem::GetInstance()
#define PHYSICS(actor) thePhysicsSystem.Get(actor)

/*! \class Physics
 *  \brief Manages physic effects */
UPDATABLE_SYSTEM(Physics)

public:
    /*! \brief ?
     *  \param pc
     *  \param m */
	static void addMoment(PhysicsComponent* pc, float m);
};
