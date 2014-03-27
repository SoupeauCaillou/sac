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

#include "NetworkSystem.h"
#include "../api/NetworkAPI.h"
#include "../base/EntityManager.h"
#include <queue>

#define USE_SYSTEM_IDX 1

struct StatusCache {
    std::map<std::string, uint8_t*> components;
};
std::map<Entity, StatusCache> statusCache;
typedef std::map<std::string, uint8_t*>::iterator CacheIt;

#if SAC_DEBUG
static unsigned bytesSentLastSec, bytesReceivedLastSec;
static float counterTime;
#define SEND(pkt) do { networkAPI->sendPacket(pkt); bytesSentLastSec += pkt.size; ++theNetworkSystem.packetSent; } while (false)
#else
#define SEND(pkt) do { networkAPI->sendPacket(pkt); } while (false)
#endif

#if SAC_ANDROID
    #define GUID_TAG 0
#else
    #include "api/linux/NetworkAPILinuxImpl.h"
    #define GUID_TAG (static_cast<NetworkAPILinuxImpl*>(networkAPI))->guidTag()
#endif

struct NetworkComponentPriv : NetworkComponent {
    NetworkComponentPriv() : NetworkComponent(), guid(0), entityExistsGlobally(false) /* ownedLocally(true)*/ {}
    unsigned int guid; // global unique id
    bool entityExistsGlobally; //, ownedLocally;
    std::queue<NetworkPacket> packetToProcess;
};

struct NetworkMessageHeader {
    enum Type {
        HandShake = 0,
        CreateEntity,
        DeleteEntity,
        UpdateEntity,
#if 0
        ChangeEntityOwner,
#endif
    } type;
    unsigned int entityGuid;

    union {
        struct {
            unsigned int guidTag;
        } HANDSHAKE;
        struct {

        } CREATE;
        struct {

        } _DELETE;
        struct {

        } UPDATE;
#if 0
        struct {
            unsigned newOwner; // currently : 0 = master, 1 = client
        } CHANGE_OWNERSHIP;
#endif
    };
};

INSTANCE_IMPL(NetworkSystem);


NetworkSystem::NetworkSystem() : ComponentSystemImpl<NetworkComponent>("Network"), networkAPI(0) {
    nextGuid = 1;

    NetworkComponentPriv nc;
    componentSerializer.add(new VectorProperty<std::string>(HASH("sync", 0x0), OFFSET(sync, nc)));

#if SAC_DEBUG
    bytesSentLastSec = bytesReceivedLastSec = 0;
    packetSent = packetRcvd = 0;
    bytesSent = bytesReceived = 0;
    counterTime = ulRate= dlRate = 0;
#endif
}

void NetworkSystem::DoUpdate(float dt) {
    if (!networkAPI)
        return;

    bool isHosting = false;

    #if ! SAC_ANDROID
    isHosting = ((static_cast<NetworkAPILinuxImpl*>(networkAPI))->getStatus() == NetworkStatus::InRoomAsMaster);
    #endif

    // Pull packets from networkAPI
    // We want to retrieve received packets, and then treat them as needed.
    {
        NetworkPacket pkt;
        while ((pkt = networkAPI->pullReceivedPacket()).size) {
#if SAC_DEBUG
            bytesReceivedLastSec += pkt.size;
            ++packetRcvd;
#endif
            NetworkMessageHeader* header = (NetworkMessageHeader*) pkt.data;

            // if I'm the server, forward this packet to other clients
            if (isHosting) {
                SEND(pkt);
            }

            switch (header->type) {
                case NetworkMessageHeader::CreateEntity: {
                    const char* name = (char*) (pkt.data + sizeof(NetworkMessageHeader));
                    LOGT("Fixme");
                    Entity e = theEntityManager.CreateEntity(Murmur::RuntimeHash(name));
                    ADD_COMPONENT(e, Network);
                    NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*>(NETWORK(e));
                    LOGV(1, "Received CREATE_ENTITY msg (guid: " << header->entityGuid << ")");
                    nc->guid = header->entityGuid;
                    LOGV(1, "Create entity :" << e << " / " << nc->guid);
                    break;
                }
                case NetworkMessageHeader::DeleteEntity: {
                    LOGV(1, "Received DELETE_ENTITY msg (guid: " << header->entityGuid << ")");
                    Entity e = guidToEntity(header->entityGuid);
                    if (e) {
                        theEntityManager.DeleteEntity(e);
                    } else {
                        LOGE("Unable to find entity to delete");
                    }
                    break;
                }
                case NetworkMessageHeader::UpdateEntity: {
                    LOGV(1, "Received UPDATE_ENTITY msg (guid: " << header->entityGuid << ")");
                    NetworkComponentPriv* nc = guidToComponent(header->entityGuid);
                    if (nc) {
                        // TODO
                        if (nc->guid & GUID_TAG) {
                            // do not apply
                        } else {
                            nc->packetToProcess.push(pkt);
                        }
                    }
                    break;
                }
#if 0
                case NetworkMessageHeader::ChangeEntityOwner: {
                    LOGI("Received CHANGE_OWNER msg");
                    NetworkComponentPriv* nc = guidToComponent(header->entityGuid);
                    if (nc) {
                        if (networkAPI->amIGameMaster()) {
                            nc->ownedLocally = (header->CHANGE_OWNERSHIP.newOwner == 0);
                        } else {
                            nc->ownedLocally = (header->CHANGE_OWNERSHIP.newOwner == 1);
                        }
                        nc->entityExistsGlobally = true;
                        LOGI("guid " << header->entityGuid << " changed owner : " << header->CHANGE_OWNERSHIP.newOwner << " (isLocal: " << nc->ownedLocally << ")");
                    }
                    break;
                }
#endif
                default: {
                    break;
                }
            }
        }
    }
    // Process update type packets received
    {
#if USE_SYSTEM_IDX
        const auto& name2ptr = ComponentSystem::registeredSystems();
#else
        uint8_t temp[1024];
#endif

        FOR_EACH_ENTITY_COMPONENT(Network, e, ncc)
            NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (ncc);

            while (!nc->packetToProcess.empty()) {
                NetworkPacket pkt = nc->packetToProcess.front();
                int index = sizeof(NetworkMessageHeader);
                while (index < pkt.size) {
#if USE_SYSTEM_IDX
                    auto it = name2ptr.begin();
                    std::advance(it, pkt.data[index++]);
                    ComponentSystem* system = it->second;
#else
                    uint8_t nameLength = pkt.data[index++];
                    memcpy(temp, &pkt.data[index], nameLength);
                    temp[nameLength] = '\0';
                    index += nameLength;
                    ComponentSystem* system = ComponentSystem::Named((const char*)temp);
#endif
                    int size;
                    memcpy(&size, &pkt.data[index], 4);
                    index += 4;
                    index += system->deserialize(e, &pkt.data[index], size);
                }
                 nc->packetToProcess.pop();
            }
        END_FOR_EACH()
    }

    // Process local entities : send required update to others
    {
        FOR_EACH_ENTITY_COMPONENT(Network, e, nc)
            updateEntity(e, nc, dt, true);
        END_FOR_EACH()
        FOR_EACH_ENTITY_COMPONENT(Network, e, nc)
            updateEntity(e, nc, dt, false);
        END_FOR_EACH()
    }

    // Forward entity deletion
    LOGI_IF(!deletedEntities.empty(), "Dispatch " << deletedEntities.size() << " deletion");
    std::for_each(deletedEntities.begin(), deletedEntities.end(), [this] (unsigned int guid) -> void {
        uint8_t temp[1024];
        NetworkPacket pkt;
        NetworkMessageHeader* header = (NetworkMessageHeader*)temp;
        header->type = NetworkMessageHeader::DeleteEntity;
        header->entityGuid = guid;
        pkt.size = sizeof(NetworkMessageHeader);
        pkt.data = temp;
        SEND(pkt);
    });
    deletedEntities.clear();

#if SAC_DEBUG
    float diff = TimeUtil::GetTime() - counterTime;
    if (diff >= 1.0) {
        bytesSent += bytesSentLastSec;
        bytesReceived += bytesReceivedLastSec;
        ulRate = bytesSentLastSec / diff;
        dlRate = bytesReceivedLastSec / diff;
        counterTime = TimeUtil::GetTime();
        LOGI_EVERY_N(10, "Network statititics: DL=" << bytesReceivedLastSec/1024.<< " kB/s UL=" << bytesSentLastSec/1024. << " kB/s");
        bytesSentLastSec = bytesReceivedLastSec = 0;
    }
#endif
}

void NetworkSystem::updateEntity(Entity e, NetworkComponent* comp, float, bool onlyCreate) {
    static uint8_t temp[1024];
    NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (comp);

    if (nc->guid != 0 && !(nc->guid & GUID_TAG))
        return;
    if (nc->sync.empty())
        return;

    if (!nc->entityExistsGlobally && onlyCreate) {
        // later nc->entityExistsGlobally = true;
        nc->guid = (nextGuid++) | GUID_TAG;
        LOGT("Fime");
        #if 0
        const std::string& name = theEntityManager.entityName(e);
        NetworkPacket pkt;
        NetworkMessageHeader* header = (NetworkMessageHeader*)temp;
        header->type = NetworkMessageHeader::CreateEntity;
        header->entityGuid = nc->guid;
        pkt.size = sizeof(NetworkMessageHeader) + name.length() + 1;
        std::strcpy ((char*)&temp[sizeof(NetworkMessageHeader)], name.c_str());
        pkt.data = temp;
        SEND(pkt);
        LOGV(1, "NOTIFY create : " << e << "/" << nc->guid << '/' << theEntityManager.entityName(e));
        #endif
    }
    if (onlyCreate)
        return;
    StatusCache& cache = statusCache[e];

    NetworkPacket pkt;
    // build packet header
    NetworkMessageHeader* header = (NetworkMessageHeader*)temp;
    header->type = NetworkMessageHeader::UpdateEntity;
    header->entityGuid = nc->guid;
    pkt.size = sizeof(NetworkMessageHeader);

#if USE_SYSTEM_IDX
    const auto& name2ptr = ComponentSystem::registeredSystems();
#endif
    // browse systems to share on network for this entity (of course, batching this would make a lot of sense)
    for (auto& name : nc->sync) {
        // time to update

#if USE_SYSTEM_IDX
        auto it = name2ptr.find(name);
        ComponentSystem* system = it->second;
        uint8_t idx = std::distance(name2ptr.begin(), it);
#else
        ComponentSystem* system = ComponentSystem::Named(name);
#endif
        // find cache entry, if any
        uint8_t* cacheEntry = 0;

        CacheIt c = cache.components.find(name);
        if (c != cache.components.end()) {
            cacheEntry = c->second;
        }

        uint8_t* out;
        int size = system->serialize(e, &out, cacheEntry);
        if (size > 0 || !nc->entityExistsGlobally) {
#if USE_SYSTEM_IDX
            temp[pkt.size++] = idx;
#else
            uint8_t nameLength = strlen(name.c_str());
            temp[pkt.size++] = nameLength;
            memcpy(&temp[pkt.size], name.c_str(), nameLength);
            pkt.size += nameLength;
#endif
            memcpy(&temp[pkt.size], &size, 4);
            pkt.size += 4;
            if (size > 0) {
                memcpy(&temp[pkt.size], out, size);
                pkt.size += size;

                cacheEntry = system->saveComponent(e, cacheEntry);
                if (c == cache.components.end()) {
                    cache.components.insert(std::make_pair(name, cacheEntry));
                }
                delete[] out;
            }
        }
    }
    if (!nc->entityExistsGlobally || pkt.size > (int)sizeof(NetworkMessageHeader)) {
        nc->entityExistsGlobally = true;
        // finish up packet
        pkt.data = temp;
        SEND(pkt);
    }
}


void NetworkSystem::Delete(Entity e) {
#if SAC_USE_VECTOR_STORAGE
    LOGT("TODO " << e);
#else
    // mark the entity as deleted
    auto it = components.find(e);

    NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (it->second);

    if (nc->guid & GUID_TAG) {
        deletedEntities.push_back(nc->guid);
    }

    // delete it here
    ComponentSystemImpl<NetworkComponent>::Delete(e);
#endif
}

NetworkComponent* NetworkSystem::CreateComponent() {
    return new NetworkComponentPriv(); // ahah!
}

NetworkComponentPriv* NetworkSystem::guidToComponent(unsigned int guid) {
    FOR_EACH_COMPONENT(Network, ncc)
        NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (ncc);
        if (nc->guid == guid)
            return nc;
    END_FOR_EACH()
    //LOGE("Did not find entity with guid: %u", guid);
    return 0;
}

Entity NetworkSystem::guidToEntity(unsigned int guid) {
    if (guid == 0)
        return 0;
    FOR_EACH_ENTITY_COMPONENT(Network, e, ncc)
        NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (ncc);
        if (nc->guid == guid)
            return e;
    END_FOR_EACH()
    LOGE("Did not find entity with guid: " << guid);
    return 0;
}

bool NetworkSystem::isOwnedLocally(Entity e) {
    const NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (Get(e));
    return (nc->guid & GUID_TAG);
}

void NetworkSystem::deleteAllNonLocalEntities() {
    int count = 0;
    FOR_EACH_ENTITY_COMPONENT(Network, e, ncc)
        NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (ncc);
        if (!(nc->guid & GUID_TAG)) {
            theEntityManager.DeleteEntity(e);
            count++;
        }
    END_FOR_EACH()
    LOGI("Removed " << count << " non local entities");
}

unsigned int NetworkSystem::entityToGuid(Entity e) {
    if (e <= 0)
        return 0;
    NetworkComponent* ncc = Get(e, false);
    if (ncc == 0) {
        LOGF("Entity " << e << " has no network component");
        return 0;
    }
    NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (ncc);
    return nc->guid;
}
