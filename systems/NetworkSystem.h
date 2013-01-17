/*!
 * \file NetworkSystem.h
 * \brief 
 * \author Pierre-Eric Pelloux-Prayer
 * \author Gautier Pelloux-Prayer
 */
#pragma once

#include "System.h"
class NetworkAPI;
struct NetworkComponentPriv;

/*! \struct NetworkComponent
 *  \brief ? */
struct NetworkComponent {
    NetworkComponent() : newOwnerShipRequest(-1) {
        systemUpdatePeriod["network"] = 0;
    }
    std::map<std::string, float> systemUpdatePeriod; //!< ?
    int newOwnerShipRequest; //!< ?
};

#define theNetworkSystem NetworkSystem::GetInstance()
#define NETWORK(e) theNetworkSystem.Get(e)

/*! \class Network
 *  \brief ? */
UPDATABLE_SYSTEM(Network)

public:
    /*! \brief */
    void deleteAllNonLocalEntities();

    /*! \brief
     *  \param e
     *  \return */
    unsigned int entityToGuid(Entity e);

    /*! \brief
     *  \param guid
     *  \return */
    Entity guidToEntity(unsigned int guid);
public:
    NetworkAPI* networkAPI; //!< ?
    unsigned bytesSent, bytesReceived; //!< ?
    float ulRate, dlRate; //!< ?

protected:
    /*! \brief */
    NetworkComponent* CreateComponent();
private:
    /*! \brief
     *  \param guid
     *  \return */
    NetworkComponentPriv* guidToComponent(unsigned int guid);

    /*! \brief
     *  \param e
     *  \param c
     *  \param dt
     *  \return */
    void updateEntity(Entity e, NetworkComponent* c, float dt);
    unsigned int nextGuid; //!< ?
};
