#include "NetworkSystem.h"
#include "../api/NetworkAPI.h"
#include "../base/EntityManager.h"
#include <queue>

struct StatusCache {
    std::map<std::string, uint8_t*> components;
};
std::map<Entity, StatusCache> statusCache;
typedef std::map<std::string, uint8_t*>::iterator CacheIt;

static unsigned bytesSentLastSec, bytesReceivedLastSec;
static float counterTime;

#define SEND(pkt) networkAPI->sendPacket(pkt); bytesSentLastSec += pkt.size

struct NetworkComponentPriv : NetworkComponent {
    NetworkComponentPriv() : NetworkComponent(), guid(0), entityExistsGlobally(false), ownedLocally(true) {}
    unsigned int guid; // global unique id
    bool entityExistsGlobally, ownedLocally;
    std::map<std::string, float> lastUpdateAccum;
    std::queue<NetworkPacket> packetToProcess;
};

struct NetworkMessageHeader {
    enum Type {
        HandShake = 0,
        CreateEntity,
        DeleteEntity,
        UpdateEntity,
        ChangeEntityOwner,
    } type;
    unsigned int entityGuid;

    union {
        struct {
            unsigned int nonce;
        } HANDSHAKE;
        struct {

        } CREATE;
        struct {

        } _DELETE;
        struct {

        } UPDATE;
        struct {
            unsigned newOwner; // currently : 0 = master, 1 = client
        } CHANGE_OWNERSHIP;
    };
};

static void sendHandShakePacket(NetworkAPI* net, unsigned nonce);

INSTANCE_IMPL(NetworkSystem);

 unsigned myNonce;
 bool hsDone;
NetworkSystem::NetworkSystem() : ComponentSystemImpl<NetworkComponent>("network"), networkAPI(0) {
    /* nothing saved (?!) */
    nextGuid = 0;
    hsDone = false;
    myNonce = MathUtil::RandomInt(65000);

    NetworkComponentPriv nc;
    componentSerializer.add(new MapProperty<std::string, float>(OFFSET(systemUpdatePeriod, nc)));
}


void NetworkSystem::DoUpdate(float dt) {
    if (!networkAPI)
        return;
    if (!networkAPI->isConnectedToAnotherPlayer()) {
        counterTime = TimeUtil::GetTime();
        bytesSent = bytesReceived = bytesSentLastSec = bytesReceivedLastSec = 0;
        return;
    }
    // Pull packets from networkAPI
    {
        NetworkPacket pkt;
        while ((pkt = networkAPI->pullReceivedPacket()).size) {
            bytesReceivedLastSec += pkt.size;
            NetworkMessageHeader* header = (NetworkMessageHeader*) pkt.data;
            switch (header->type) {
                case NetworkMessageHeader::HandShake: {
                    LOGV(1, "Received HANDSHAKE msg / " << header->HANDSHAKE.nonce)
                    if (header->HANDSHAKE.nonce == myNonce) {
                        LOGI("Handshake done")
                        hsDone = true;
                    } else {
                        sendHandShakePacket(networkAPI, header->HANDSHAKE.nonce);
                    }
                    break;
                }
                case NetworkMessageHeader::CreateEntity: {
                    LOGV(1, "Received CREATE_ENTITY msg")
                    Entity e = theEntityManager.CreateEntity();
                    ADD_COMPONENT(e, Network);
                    NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*>(NETWORK(e));
                    nc->guid = header->entityGuid;
                    nc->ownedLocally = false;
                    LOGV(1, "Create entity :" << e << " / " << nc->guid)
                    break;
                }
                case NetworkMessageHeader::DeleteEntity: {
                    LOGV(1, "Received DELETE_ENTITY msg")
                    Entity e = guidToEntity(header->entityGuid);
                    if (e) {
                        theEntityManager.DeleteEntity(e);
                    }
                    break;
                }
                case NetworkMessageHeader::UpdateEntity: {
                    LOGV(2, "Received UPDATE_ENTITY msg")
                    NetworkComponentPriv* nc = guidToComponent(header->entityGuid);
                    if (nc) {
                        nc->packetToProcess.push(pkt);
                    }
                    break;
                }
                case NetworkMessageHeader::ChangeEntityOwner: {
                    LOGV(1, "Received CHANGE_OWNER msg")
                    NetworkComponentPriv* nc = guidToComponent(header->entityGuid);
                    if (nc) {
                        if (networkAPI->amIGameMaster()) {
                            nc->ownedLocally = (header->CHANGE_OWNERSHIP.newOwner == 0);
                        } else {
                            nc->ownedLocally = (header->CHANGE_OWNERSHIP.newOwner == 1);
                        }
                        nc->entityExistsGlobally = true;
                        LOGV(1, "guid " << header->entityGuid << " changed owner : " << header->CHANGE_OWNERSHIP.newOwner << " (isLocal: " << nc->ownedLocally << ")")
                    }
                    break;
                }
            }
        }
    }

    if (!hsDone) {
        sendHandShakePacket(networkAPI, myNonce);
        return;
    }

    // Process update type packets received
    {
        uint8_t temp[1024];
        FOR_EACH_ENTITY_COMPONENT(Network, e, ncc)
            NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (ncc);

            if (0 && nc->ownedLocally)
                continue;
            while (!nc->packetToProcess.empty()) {
                NetworkPacket pkt = nc->packetToProcess.front();
                int index = sizeof(NetworkMessageHeader);
                while (index < pkt.size) {
                    uint8_t nameLength = pkt.data[index++];
                    memcpy(temp, &pkt.data[index], nameLength);
                    temp[nameLength] = '\0';
                    index += nameLength;
                    int size;
                    memcpy(&size, &pkt.data[index], 4);
                    index += 4;
                    ComponentSystem* system = ComponentSystem::Named((const char*)temp);
                    index += system->deserialize(e, &pkt.data[index], size);
                }
                 nc->packetToProcess.pop();
            }
        }
    }

    // Process local entities : send required update to others
    {
        FOR_EACH_ENTITY_COMPONENT(Network, e, nc)
            updateEntity(e, nc, dt);
        }
    }

    float diff = TimeUtil::GetTime() - counterTime;
    if (diff >= 1.0) {
        bytesSent += bytesSentLastSec;
        bytesReceived += bytesReceivedLastSec;
        ulRate = bytesSentLastSec / diff;
        dlRate = bytesReceivedLastSec / diff;
        counterTime = TimeUtil::GetTime();
        LOGI("Network statititics: DL=" << bytesReceivedLastSec/1024.<< " kB/s UL=" << bytesSentLastSec/1024. << " kB/s")
        bytesSentLastSec = bytesReceivedLastSec = 0;
    }
}

void NetworkSystem::updateEntity(Entity e, NetworkComponent* comp, float dt) {
    static uint8_t temp[1024];
    NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (comp);

    if (!nc->ownedLocally || nc->systemUpdatePeriod.empty())
        return;

    if (!nc->entityExistsGlobally) {
        // later nc->entityExistsGlobally = true;
        nc->guid = (nextGuid += 2);
        if (!networkAPI->amIGameMaster()) {
            nc->guid |= 0x1;
        }
        NetworkPacket pkt;
        NetworkMessageHeader* header = (NetworkMessageHeader*)temp;
        header->type = NetworkMessageHeader::CreateEntity;
        header->entityGuid = nc->guid;
        pkt.size = sizeof(NetworkMessageHeader);
        pkt.data = temp;
        SEND(pkt);
        LOGV(1, "NOTIFY create : " << e << "/" << nc->guid)
    }
    StatusCache& cache = statusCache[e];

    NetworkPacket pkt;
    // build packet header
    NetworkMessageHeader* header = (NetworkMessageHeader*)temp;
    header->type = NetworkMessageHeader::UpdateEntity;
    header->entityGuid = nc->guid;
    pkt.size = sizeof(NetworkMessageHeader);

    // browse systems to share on network for this entity (of course, batching this would make a lot of sense)
    for (std::map<std::string, float>::iterator jt = nc->systemUpdatePeriod.begin(); jt!=nc->systemUpdatePeriod.end(); ++jt) {
        float& accum = nc->lastUpdateAccum[jt->first];

        if (accum >= 0)
            accum += dt;
        // time to update
        if (accum >= jt->second || !nc->entityExistsGlobally) {
            ComponentSystem* system = ComponentSystem::Named(jt->first);
            // find cache entry, if any
            uint8_t* cacheEntry = 0;
            #if 1
            CacheIt c = cache.components.find(jt->first);
            if (c == cache.components.end()) {
                // cache.components.insert(std::make_pair(jt->first, system->saveComponent(e)));
                LOGW("No cache found for component: " << jt->first)
            } else {
                cacheEntry = c->second;
            }
            #endif
            uint8_t* out;
            int size = system->serialize(e, &out, cacheEntry);
            if (size > 0) {
                //LOG_EVERY_N(INFO, 250) << jt->first << " size: " << size;
                uint8_t nameLength = strlen(jt->first.c_str());
                temp[pkt.size++] = nameLength;
                memcpy(&temp[pkt.size], jt->first.c_str(), nameLength);
                pkt.size += nameLength;
                memcpy(&temp[pkt.size], &size, 4);
                pkt.size += 4;
                memcpy(&temp[pkt.size], out, size);
                pkt.size += size;

                cacheEntry = system->saveComponent(e, cacheEntry);
                if (c == cache.components.end()) {
                    cache.components.insert(std::make_pair(jt->first, cacheEntry));
                }
                delete[] out;
            }
            accum = 0;
        }
        // if peridocity <= 0 => update only once
        if (jt->second <= 0) {
            accum = -1;
        }
    }
    // LOG_EVERY_N(INFO, 500) << pkt.size << " b for entity " << theEntityManager.entityName(e);
    nc->entityExistsGlobally = true;
    // finish up packet
    pkt.data = temp;
    SEND(pkt);

    if (nc->newOwnerShipRequest >= 0) {
        uint8_t temp[64];
        NetworkPacket pkt;
        NetworkMessageHeader* header = (NetworkMessageHeader*)temp;
        header->type = NetworkMessageHeader::ChangeEntityOwner;
        header->entityGuid = nc->guid;
        header->CHANGE_OWNERSHIP.newOwner = nc->newOwnerShipRequest;
        pkt.size = sizeof(NetworkMessageHeader);
        pkt.data = temp;
        SEND(pkt);
        if (networkAPI->amIGameMaster()) {
            nc->ownedLocally = (nc->newOwnerShipRequest==0);
        } else {
            nc->ownedLocally = (nc->newOwnerShipRequest==1);
        }
        nc->newOwnerShipRequest = -1;
        LOGV(1, "Send change ownrship request for entity " << e << "/" << nc->guid << " :" << header->CHANGE_OWNERSHIP.newOwner << "(is local: " << nc->ownedLocally << ")")
    }
}

NetworkComponent* NetworkSystem::CreateComponent() {
    return new NetworkComponentPriv(); // ahah!
}

NetworkComponentPriv* NetworkSystem::guidToComponent(unsigned int guid) {
    FOR_EACH_COMPONENT(Network, ncc)
        NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (ncc);
        if (nc->guid == guid)
            return nc;
    }
    //LOGE("Did not find entity with guid: %u", guid);
    return 0;
}

Entity NetworkSystem::guidToEntity(unsigned int guid) {
    FOR_EACH_ENTITY_COMPONENT(Network, e, ncc)
        NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (ncc);
        if (nc->guid == guid)
            return e;
    }
    LOGE("Did not find entity with guid: " << guid)
    return 0;
}

void NetworkSystem::deleteAllNonLocalEntities() {
    int count = 0;
    FOR_EACH_ENTITY_COMPONENT(Network, e, ncc)
        NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (ncc);
        if (!nc->ownedLocally) {
            theEntityManager.DeleteEntity(e);
            count++;
        }
    }
    LOGI("Removed " << count << " non local entities")
}

unsigned int NetworkSystem::entityToGuid(Entity e) {
    NetworkComponent* ncc = Get(e, false);
    if (ncc == 0) {
        LOGF("Entity " << e << " has no network component")
        return 0;
    }
    NetworkComponentPriv* nc = static_cast<NetworkComponentPriv*> (ncc);
    return nc->guid;
}

static void sendHandShakePacket(NetworkAPI* networkAPI, unsigned nonce) {
    uint8_t temp[64];
    NetworkPacket pkt;
    NetworkMessageHeader* header = (NetworkMessageHeader*)temp;
    header->type = NetworkMessageHeader::HandShake;
    header->HANDSHAKE.nonce = nonce;
    LOGV(1, "Send handshake packet :" << nonce)
    pkt.size = sizeof(NetworkMessageHeader);
    pkt.data = temp;
    SEND(pkt);
}

#ifdef SAC_INGAME_EDITORS
void NetworkSystem::addEntityPropertiesToBar(Entity, TwBar*) {

}
#endif
