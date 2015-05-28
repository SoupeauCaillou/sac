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

#include "NetworkSystem.h"
#include "../api/NetworkAPI.h"
#include "../base/EntityManager.h"
#include <queue>
#include <algorithm>
#include "base/TimeUtil.h"
#include "util/SerializerProperty.h"
#include "util/Random.h"

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
    hash_t entityHash;

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


NetworkSystem::NetworkSystem() : ComponentSystemImpl<NetworkComponent>(HASH("Network", 0x3b7514b1), ComponentType::Complex), networkAPI(0) {
    nextGuid = 1;

    NetworkComponent nc;
    componentSerializer.add(new Property<int>(HASH("guid", 0x6eafbf16), OFFSET(guid, nc)));
    componentSerializer.add(new VectorProperty<std::string>(HASH("sync", 0x31a5991f), OFFSET(sync, nc)));

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
                // SEND(pkt);
            }

            switch (header->type) {
#if 0
                case NetworkMessageHeader::CreateEntity: {
                    hash_t hash;
                    memcpy(&hash, pkt.data + sizeof(NetworkMessageHeader), sizeof(hash_t));
                    Entity e = theEntityManager.CreateEntity(hash);
                    ADD_COMPONENT(e, Network);
                    NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*>(NETWORK(e));
                    LOGV(1, "Received CREATE_ENTITY msg (guid: " << header->entityGuid << ")");
                    nc->guid = header->entityGuid;
                    LOGV(1, "Create entity :" << e << " / " << nc->guid);
                    break;
                }
#endif
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
                    LOGI_EVERY_N(10, "Received UPDATE_ENTITY msg (guid: " << header->entityGuid << ")");
                    NetworkComponent* nc = guidToComponent(header->entityGuid);
                    if (!nc) {
                        /* create entity */
                        Entity e = theEntityManager.CreateEntity(header->entityHash);
                        ADD_COMPONENT(e, Network);
                        nc = NETWORK(e);
                        nc->guid = header->entityGuid;
                    }

                    if (nc->guid & GUID_TAG) {
                        // do not apply our own packets
                    } else {
                        nc->packetToProcess.push(pkt);
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

        FOR_EACH_ENTITY_COMPONENT(Network, e, nc)

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
                    if (!system->hasComponent(e)) {
                        LOGI("Adding component " << INV_HASH(system->getId()) << " to " << e);
                        theEntityManager.AddComponent(e, system);
                    }
                    int size;
                    memcpy(&size, &pkt.data[index], 4);
                    index += 4;
                    index += system->deserialize(e, &pkt.data[index], size);
                }
                nc->packetToProcess.pop();
            }
        END_FOR_EACH()
    }

    if (! GUID_TAG )
        return;


    // Process local entities : send required update to others
    {
        /*FOR_EACH_ENTITY_COMPONENT(Network, e, nc)
            updateEntity(e, nc, dt, true);
        END_FOR_EACH()*/
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

void NetworkSystem::updateEntity(Entity e, NetworkComponent* nc, float, bool ) {
    static uint8_t temp[1024];

    if (nc->guid != 0 && !(nc->guid & GUID_TAG))
        return;
    if (nc->sync.empty())
        return;

    if (nc->guid == 0) {
        nc->guid = (nextGuid++) | GUID_TAG;
    }

#if 0
    if (!nc->entityExistsGlobally && onlyCreate) {
        // later nc->entityExistsGlobally = true;
        nc->guid = (nextGuid++) | GUID_TAG;
        const hash_t h = theEntityManager.entityHash(e);
        NetworkPacket pkt;
        NetworkMessageHeader* header = (NetworkMessageHeader*)temp;
        header->type = NetworkMessageHeader::CreateEntity;
        header->entityGuid = nc->guid;
        pkt.size = sizeof(NetworkMessageHeader) + sizeof(hash_t);
        memcpy(&temp[sizeof(NetworkMessageHeader)], &h, sizeof(h));
        pkt.data = temp;
        SEND(pkt);
        LOGV(1, "NOTIFY create : " << e << "/" << nc->guid << '/' << theEntityManager.entityName(e));
    }
    if (onlyCreate)
        return;
#endif
    StatusCache& cache = statusCache[e];

    NetworkPacket pkt;
    // build packet header
    NetworkMessageHeader* header = (NetworkMessageHeader*)temp;
    header->type = NetworkMessageHeader::UpdateEntity;
    header->entityGuid = nc->guid;
    header->entityHash = theEntityManager.entityHash(e);
    pkt.size = sizeof(NetworkMessageHeader);

#if USE_SYSTEM_IDX
    const auto& name2ptr = ComponentSystem::registeredSystems();
#endif
    // browse systems to share on network for this entity (of course, batching this would make a lot of sense)
    for (auto& name : nc->sync) {
        // time to update

#if USE_SYSTEM_IDX
        const auto id = Murmur::RuntimeHash(name.c_str());
        const auto it = name2ptr.find(id);
        ComponentSystem* system = (*it).second;
        uint8_t idx = std::distance(name2ptr.begin(), it);
#else
        ComponentSystem* system = ComponentSystem::GetById(name);
#endif
        // find cache entry, if any
        uint8_t* cacheEntry = 0;

        CacheIt c = cache.components.find(name);
        if (c != cache.components.end()) {
            cacheEntry = c->second;
        }

        int forceFullUpdate = Random::Float(0, 1) >= 0.99;
        uint8_t* out;
        int size = system->serialize(e, &out, forceFullUpdate ? 0 : cacheEntry);
        {
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


void NetworkSystem::Delete(Entity ) {

#if 0
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

/*NetworkComponent* NetworkSystem::CreateComponent() {
    auto* nc = new NetworkComponentPriv(); // ahah!
    nc->guid = (nextGuid++) | GUID_TAG;
    return nc;
}*/

NetworkComponent* NetworkSystem::guidToComponent(unsigned int guid) {
    FOR_EACH_COMPONENT(Network, nc)
        if (nc->guid == guid)
            return nc;
    END_FOR_EACH()
    //LOGE("Did not find entity with guid: %u", guid);
    return 0;
}

Entity NetworkSystem::guidToEntity(unsigned int guid) {
    if (guid == 0)
        return 0;
    FOR_EACH_ENTITY_COMPONENT(Network, e, nc)
        if (nc->guid == guid)
            return e;
    END_FOR_EACH()
    LOGE("Did not find entity with guid: " << guid);
    return 0;
}

bool NetworkSystem::isOwnedLocally(Entity e) {
    const NetworkComponent* nc = Get(e);
    return (nc->guid & GUID_TAG);
}

void NetworkSystem::deleteAllNonLocalEntities() {
    int count = 0;
    FOR_EACH_ENTITY_COMPONENT(Network, e, nc)
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
    NetworkComponent* nc = Get(e, false);
    if (nc == 0) {
        LOGF("Entity " << e << " has no network component");
        return 0;
    }
    return nc->guid;
}

#endif

