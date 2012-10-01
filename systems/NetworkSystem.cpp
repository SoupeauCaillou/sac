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
#include "NetworkSystem.h"
#include "../api/NetworkAPI.h"
#include "../base/EntityManager.h"
#include <queue>

struct NetworkComponentPriv : NetworkComponent {
    NetworkComponentPriv() : NetworkComponent(), guid(0), entityExistsGlobally(false), ownedLocally(true) {}
    unsigned int guid; // global unique id
    bool entityExistsGlobally, ownedLocally;
    std::map<std::string, float> lastUpdateAccum;
    std::queue<NetworkPacket> packetToProcess;
};

struct NetworkMessageHeader {
    enum Type {
        CreateEntity,
        DeleteEntity,
        UpdateEntity
    } type;
    unsigned int entityGuid;

    union {
        struct {

        } CREATE;
        struct {

        } DELETE;
        struct {

        } UPDATE;
    };
};

static NetworkComponentPriv* guidToComponent(unsigned int guid);

INSTANCE_IMPL(NetworkSystem);
 
NetworkSystem::NetworkSystem() : ComponentSystemImpl<NetworkComponent>("network"), networkAPI(0) { 
 
}

void NetworkSystem::DoUpdate(float dt) {
    if (!networkAPI)
        return;
    // Pull packets from networkAPI
    {
        NetworkPacket pkt;
        while ((pkt = networkAPI->pullReceivedPacket()).size) {
            NetworkMessageHeader* header = (NetworkMessageHeader*) pkt.data;
            switch (header->type) {
                case NetworkMessageHeader::CreateEntity: {
                    Entity e = theEntityManager.CreateEntity();
                    ADD_COMPONENT(e, Network);
                    NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*>(NETWORK(e));
                    nc->guid = header->entityGuid;
                    nc->ownedLocally = false;
                    break;
                }
                case NetworkMessageHeader::DeleteEntity: {
                    Entity e = guidToEntity(header->entityGuid);
                    if (e) {
                        theEntityManager.DeleteEntity(e);
                    }
                    break;
                }
                case NetworkMessageHeader::UpdateEntity: {
                    NetworkComponentPriv* nc = guidToComponent(header->entityGuid);
                    if (nc) {
                        nc->packetToProcess.push(pkt);
                    }
                    break;
                }
            }
        }
    }

    // Process update type packets received
    {
        uint8_t temp[1024];
        for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
            Entity e = it->first;
            NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (it->second);
    
            if (nc->ownedLocally)
                continue;
            while (!nc->packetToProcess.empty()) {
                 NetworkPacket pkt = nc->packetToProcess.front();
                int index = sizeof(NetworkMessageHeader);
                while (index < pkt.size) {
                    uint8_t nameLength = pkt.data[index++];
                    memcpy(temp, &pkt.data[index], nameLength);
                    temp[nameLength] = '\0';
                    index += nameLength;
                    ComponentSystem* system = ComponentSystem::Named((const char*)temp);
                    system->deserialize(e, &temp[index], -1);
                }
                 nc->packetToProcess.pop();
            }
        }
    }

    // Process local entities : send required update to others
    {
        uint8_t temp[1024];
        for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
            Entity e = it->first;
            NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (it->second);

            if (!nc->ownedLocally || nc->systemUpdatePeriod.empty())
                continue;

            NetworkPacket pkt;
            // build packet header
            NetworkMessageHeader* header = (NetworkMessageHeader*)temp;
            header->type = NetworkMessageHeader::UpdateEntity;
            header->entityGuid = nc->guid;
            pkt.size = sizeof(NetworkMessageHeader);

            // browse systems to share on network for this entity (of course, batching this would make a lot of sense)
            for (std::map<std::string, float>::iterator jt = nc->systemUpdatePeriod.begin(); jt!=nc->systemUpdatePeriod.end(); ++jt) {
                float& accum = nc->lastUpdateAccum[jt->first];
                accum += dt;
                // time to update
                if (accum >= jt->second) {
                    ComponentSystem* system = ComponentSystem::Named(jt->first);
                    uint8_t* out;
                    int size = system->serialize(e, &out);
                    uint8_t nameLength = strlen(jt->first.c_str());
                    temp[pkt.size++] = nameLength;
                    memcpy(&temp[pkt.size], jt->first.c_str(), nameLength);
                    pkt.size += nameLength;
                    memcpy(&temp[pkt.size], out, size);
                    pkt.size += size;
                    accum = 0;
                }
            }
            // finish up packet
            pkt.data = new uint8_t[pkt.size];
            memcpy(pkt.data, temp, pkt.size);
            
            networkAPI->sendPacket(pkt);
        }
    }
}

NetworkComponent* NetworkSystem::CreateComponent() {
    return new NetworkComponentPriv(); // ahah!
}

NetworkComponentPriv* NetworkSystem::guidToComponent(unsigned int guid) {
    for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
        Entity e = it->first;
        NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (it->second);
        if (nc->guid == guid)
            return nc;
    }
    LOGE("Did not find entity with guid: %u", guid);
    return 0;
}

Entity NetworkSystem::guidToEntity(unsigned int guid) {
    for(ComponentIt it=components.begin(); it!=components.end(); ++it) {
        Entity e = it->first;
        NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (it->second);
        if (nc->guid == guid)
            return e;
    }
    LOGE("Did not find entity with guid: %u", guid);
    return 0;
}

void NetworkSystem::deleteAllNonLocalEntities() {
    int count = 0;
    for(ComponentIt it=components.begin(); it!=components.end(); ) {
        Entity e = it->first;
        NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (it->second);
        ++it;
        if (!nc->ownedLocally) {
            theEntityManager.DeleteEntity(e);
            count++;
        }
    }
    LOGI("Removed %d non local entities", count);
}
