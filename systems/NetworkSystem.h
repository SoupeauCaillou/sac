/*
 This file is part of Heriswap.

 @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
 @author Soupe au Caillou - Gautier Pelloux-Prayer

 Heriswap is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, version 3.

 Heriswap is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include "System.h"
class NetworkAPI;
class NetworkComponentPriv;

struct NetworkComponent {
    NetworkComponent() : newOwnerShipRequest(-1) {}
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
    
protected:
    NetworkComponent* CreateComponent();
private:
    NetworkComponentPriv* guidToComponent(unsigned int guid);
    void updateEntity(Entity e, NetworkComponent* c, float dt);
    unsigned int nextGuid;
};
