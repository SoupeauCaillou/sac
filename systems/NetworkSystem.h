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

#if !DISABLE_NETWORK_SYSTEM
#pragma once

#include "System.h"
#include <list>
#include <queue>
#include "../api/NetworkAPI.h"
class NetworkAPI;

struct NetworkComponent {
    NetworkComponent() : guid(0), entityExistsGlobally(false) {}
    std::vector<std::string> sync;
    /* private */
    unsigned int guid; // global unique id
    bool entityExistsGlobally; //, ownedLocally;
    std::queue<NetworkPacket> packetToProcess;
};

#define theNetworkSystem NetworkSystem::GetInstance()
#if SAC_DEBUG
#define NETWORK(e) theNetworkSystem.Get(e, true, __FILE__, __LINE__)
#else
#define NETWORK(e) theNetworkSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Network)

public:
void deleteAllNonLocalEntities();
unsigned int entityToGuid(Entity e);
Entity guidToEntity(unsigned int guid);

void Delete(Entity e) override;

bool isOwnedLocally(Entity e);

public:
NetworkAPI* networkAPI;

private:
NetworkComponent* guidToComponent(unsigned int guid);
void updateEntity(Entity e, NetworkComponent* c, float dt, bool onlyCreate);
unsigned int nextGuid;
std::list<unsigned int> deletedEntities;

#if SAC_DEBUG
public:
unsigned bytesSent, bytesReceived;
unsigned packetSent, packetRcvd;
float ulRate, dlRate;
#endif
}
;
#endif
