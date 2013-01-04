#pragma once

#include "System.h"
class NetworkAPI;
struct NetworkComponentPriv;

struct NetworkComponent {
    NetworkComponent() : newOwnerShipRequest(-1) {
        systemUpdatePeriod["network"] = 0;
    }
    std::map<std::string, float> systemUpdatePeriod;
    int newOwnerShipRequest;
};

#define theNetworkSystem NetworkSystem::GetInstance()
#define NETWORK(e) theNetworkSystem.Get(e)

UPDATABLE_SYSTEM(Network)

public:
    void deleteAllNonLocalEntities();
    unsigned int entityToGuid(Entity e);
    Entity guidToEntity(unsigned int guid);
public:
    NetworkAPI* networkAPI;
    unsigned bytesSent, bytesReceived;
    float ulRate, dlRate;

protected:
    NetworkComponent* CreateComponent();
private:
    NetworkComponentPriv* guidToComponent(unsigned int guid);
    void updateEntity(Entity e, NetworkComponent* c, float dt);
    unsigned int nextGuid;
};
