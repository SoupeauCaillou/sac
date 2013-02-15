#ifndef EMSCRIPTEN
#include "NetworkAPILinuxImpl.h"
#include <enet/enet.h>
#include <glog/logging.h>
#include "../../base/TimeUtil.h"
#include <cstring>
#include <arpa/inet.h>

struct NetworkAPILinuxImpl::NetworkAPILinuxImplDatas {
    NetworkAPILinuxImplDatas() {
        lobby.client = match.host = 0;
        lobby.peer = match.peer = 0;
    }

    struct {
        std::string nickName;
        pthread_t thread;
        std::string server;
        ENetHost * client;
        ENetPeer *peer;
    } lobby;

    struct {
        ENetHost* host;
        ENetPeer* peer;
        bool connected, masterMode;
    } match;


};

static void sendNatPunchThroughPacket(int socket, const char* addr, uint16_t port);
static NetworkPacket createNickNamePacket(const std::string& nick);
static NetworkPacket createLocalPortPacket(unsigned short port);
static ENetPacket* convertPacket(const NetworkPacket& pkt, uint32_t flags);

#define NICKNAME_PKT 1
#define PORT_PKT 2
struct LobbyPacket {
    uint8_t type, nameLength, server;
    unsigned int remoteIp;
    unsigned short remotePort, localPort;
};

NetworkAPILinuxImpl::NetworkAPILinuxImpl() {
    datas = new NetworkAPILinuxImplDatas();
    enet_initialize();
}

static void* startLobbyThread(void* p) {
    NetworkAPILinuxImpl* ptr = static_cast<NetworkAPILinuxImpl*> (p);
    ptr->runLobbyThread();
    return 0;
}

void NetworkAPILinuxImpl::runLobbyThread() {
    datas->match.connected = datas->match.masterMode = false;
    datas->lobby.client = enet_host_create (NULL /* create a client host */,
        1 /* only allow 1 outgoing connection */,
        2 /* allow up 2 channels to be used, 0 and 1 */,
        57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
        14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);
    if (datas->lobby.client == 0) {
        LOG(FATAL) << "Failed to create lobby client";
        return;
    }
    ENetAddress address;
    ENetEvent event;
    enet_address_set_host (&address, datas->lobby.server.c_str());
    address.port = 54321;

    LOG(INFO) << "Trying to connect to loby server: " << datas->lobby.server << ':' << address.port;
    datas->lobby.peer = enet_host_connect (datas->lobby.client, &address, 2, 0);
    if (enet_host_service (datas->lobby.client, &event, 50000) > 0) {
        LOG(INFO) << "Connection to lobby successful";
    } else {
        LOG(INFO) << "Connection to lobby failed";
        enet_host_destroy(datas->lobby.client);
        datas->lobby.client = 0;
        return;
    }

    // on successfull connect, send nickname


    enet_peer_send(datas->lobby.peer, 0, convertPacket(createNickNamePacket(datas->lobby.nickName), ENET_PACKET_FLAG_RELIABLE));
    enet_host_flush(datas->lobby.client);


    std::string remoteName, remoteAddr;
    uint16_t localPort, remotePort;
    bool serverMode;
    bool failure = false, gotP2PInfo = false;
    while (!failure && !gotP2PInfo) {
        int ret = enet_host_service(datas->lobby.client, &event, 0);

        if (ret < 0) {
            failure  = true;
        } else {
            switch(event.type) {
                case ENET_EVENT_TYPE_NONE :
                    break;
                case ENET_EVENT_TYPE_RECEIVE: {
                    LobbyPacket* pkt = (LobbyPacket*)event.packet->data;
                    char tmp[256], tmp2[INET_ADDRSTRLEN];
                    memcpy(tmp, &event.packet->data[sizeof(LobbyPacket)], pkt->nameLength);
                    tmp[pkt->nameLength] = '\0';
                    struct in_addr ad;
                    ad.s_addr = pkt->remoteIp;
                    inet_ntop(AF_INET, &ad.s_addr, tmp2, INET_ADDRSTRLEN);
                    remoteAddr = tmp2;
                    serverMode = pkt->server;
                    remoteName = tmp;
                    localPort = pkt->localPort;
                    remotePort = pkt->remotePort;
                    LOG(INFO) << "Got info for P2P connection (" << failure << ")";
                    gotP2PInfo = true;
                    enet_packet_destroy (event.packet);
                    break;
                }
                default:
                    break;
            }
        }
    }

    if (!failure && gotP2PInfo) {
        LOG(INFO) << "Lobby phase was successfull, trying to reach player";
        if (serverMode) {
            enet_host_destroy(datas->lobby.client);
            datas->lobby.client = 0;
            LOG(INFO) << "Starting network game in SERVER mode (remote player{name:" << remoteName << " addr:" << remoteAddr << ':' << remotePort << "; localPort:" << localPort << ')';
            datas->match.connected = connectToOtherPlayerServerMode(remoteAddr.c_str(), remotePort, localPort);
            datas->match.masterMode = datas->match.connected;
        } else {
            LOG(INFO) << "Starting network game in CLIENT mode (remote player{name:" << remoteName << " addr:" << remoteAddr << ':' << remotePort;
            datas->match.connected = connectToOtherPlayerClientMode(remoteAddr.c_str(), remotePort);
            datas->match.masterMode = false;
        }
    } else {
        enet_host_destroy(datas->lobby.client);
        datas->lobby.client = 0;
    }

}

bool NetworkAPILinuxImpl::connectToOtherPlayerServerMode(const char* addr, uint16_t remotePort, uint16_t localPort) {
    ENetAddress address;
    LOG(INFO) << "Creating server socket (" << localPort << ')';
    address.host = ENET_HOST_ANY;
    address.port = localPort;
    datas->match.host = enet_host_create (&address /* the address to bind the server host to */,
                                  32      /* allow up to 32 clients and/or outgoing connections */,
                                  2      /* allow up to 2 channels to be used, 0 and 1 */,
                                  0      /* assume any amount of incoming bandwidth */,
                                  0      /* assume any amount of outgoing bandwidth */);
    float begin = TimeUtil::getTime();
    ENetEvent event;
    while ((TimeUtil::getTime()-begin) < 5) {
        int ret = enet_host_service(datas->match.host, &event, 250);
        if (ret < 0)
            return false;
        switch(event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                datas->match.peer = event.peer;
                LOG(INFO) << "Received connection";
                return true;
            }
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy (event.packet);
                break;
            default:
                break;
        }
        if (datas->match.peer) {
            enet_peer_send(datas->match.peer, 0, convertPacket(createLocalPortPacket(0), ENET_PACKET_FLAG_RELIABLE));
        } else {
            sendNatPunchThroughPacket(datas->match.host->socket, addr, remotePort);
        }
    }
    enet_host_destroy(datas->match.host);
    datas->match.host = 0;
    LOG(ERROR) << "Fail";
    return false;
}
#include <iostream>
bool NetworkAPILinuxImpl::connectToOtherPlayerClientMode(const char* addr, uint16_t remotePort) {
    datas->match.host = enet_host_create (0,
                                  32      /* allow up to 32 clients and/or outgoing connections */,
                                  2      /* allow up to 2 channels to be used, 0 and 1 */,
                                  0      /* assume any amount of incoming bandwidth */,
                                  0      /* assume any amount of outgoing bandwidth */);
    ENetAddress address;
    enet_address_set_host (&address, addr);
    address.port = remotePort;

    datas->match.peer = enet_host_connect (datas->match.host, &address, 2, 0);

    // send local socket port
    int localPort = 0;




    float begin = TimeUtil::getTime();
    ENetEvent event;
    while ((TimeUtil::getTime() - begin)< 15) {
        int ret = enet_host_service(datas->match.host, &event, 1000);
        if (ret < 0)
            return false;
        switch(event.type) {
            case ENET_EVENT_TYPE_NONE:
                break;
            case ENET_EVENT_TYPE_CONNECT:
                datas->match.peer = event.peer;
                datas->match.connected = true;
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy (event.packet);
                if (datas->match.connected)
                    return true;
                break;
            default:
                break;
        }

        if (localPort == 0) {
            struct sockaddr_in sin;
            socklen_t addrlen = sizeof(sin);
            int ret = getsockname(datas->match.host->socket, (struct sockaddr *)&sin, &addrlen);
            if( ret == 0 && sin.sin_family == AF_INET && addrlen == sizeof(sin)) {
                localPort = ntohs(sin.sin_port);
                LOG(INFO) << "Local port: " << localPort;
                enet_peer_send(datas->lobby.peer, 0, convertPacket(createLocalPortPacket(localPort), ENET_PACKET_FLAG_RELIABLE));
                enet_host_flush(datas->lobby.client);
                enet_host_destroy(datas->lobby.client);
                datas->lobby.client = 0;
            }
        }
        if (datas->match.peer) {
            enet_peer_send(datas->match.peer, 0, convertPacket(createLocalPortPacket(0), ENET_PACKET_FLAG_RELIABLE));
        }
        /*{
            sendNatPunchThroughPacket(datas->match.host->socket, addr, remotePort);
        }*/
    }
    return datas->match.connected;
}

void NetworkAPILinuxImpl::connectToLobby(const std::string& nick, const char* addr) {
    if (datas->lobby.client) {
        return;
    }
    datas->lobby.server = addr;
    datas->lobby.nickName = nick;
    datas->lobby.nickName.resize(64);

    pthread_create(&datas->lobby.thread, 0, startLobbyThread, this);
}

bool NetworkAPILinuxImpl::isConnectedToAnotherPlayer() {
    return datas->match.connected;
}

NetworkPacket NetworkAPILinuxImpl::pullReceivedPacket() {
    NetworkPacket result;
    result.size = 0;
    if (datas->match.host == 0)
        return result;
    ENetEvent event;
    int ret = enet_host_service(datas->match.host, &event, 0);
    if (ret < 0) {
        LOG(ERROR) << "enet_host_service error";
    } else if (ret == 0) {
        // nothing received
    } else {
        switch (event.type) {
            case ENET_EVENT_TYPE_DISCONNECT:
                // probably some cleanup is needed here
                LOG(WARNING) << "Disconnect even received";
                datas->match.connected = false;
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                result.size = event.packet->dataLength;
                result.data = new uint8_t[event.packet->dataLength];
                memcpy(result.data, event.packet->data, result.size);
                enet_packet_destroy (event.packet);
                break;
            default:
                break;
        }
    }
    return result;
}

void NetworkAPILinuxImpl::sendPacket(NetworkPacket packet) {
    enet_peer_send(datas->match.peer, 0, convertPacket(packet, 0));
}

bool NetworkAPILinuxImpl::amIGameMaster() {
    return datas->match.masterMode;
}

static ENetPacket* convertPacket(const NetworkPacket& pkt, uint32_t flags) {
    return enet_packet_create(pkt.data, pkt.size, flags);
}

static NetworkPacket createNickNamePacket(const std::string& nickName) {
    NetworkPacket nickPkt;
    nickPkt.size = 2 + nickName.length();
    nickPkt.data = new uint8_t[nickPkt.size];
    nickPkt.data[0] = NICKNAME_PKT;
    nickPkt.data[1] = (uint8_t)nickName.length();
    memcpy(&nickPkt.data[2], nickName.c_str(), nickName.length());
    return nickPkt;
}

static NetworkPacket createLocalPortPacket(unsigned short port) {
    NetworkPacket portPkt;
    portPkt.size = 1 + sizeof(port);
    portPkt.data = new uint8_t[portPkt.size];
    portPkt.data[0] = PORT_PKT;
    port = htons(port);
    memcpy(&portPkt.data[1], &port, sizeof(port));
    return portPkt;
}

static void sendNatPunchThroughPacket(int socket, const char* addr, uint16_t port) {
    // send NAT punch through packet
    char tmp[4];
    struct sockaddr_in c;
    c.sin_port = htons(port);
    c.sin_family = AF_INET;

    if (inet_pton(AF_INET, addr, &c.sin_addr) != 1)
        LOG(ERROR) << "inet_pton";

    int n = sendto(socket, tmp, 4, 0, (struct sockaddr*)&c, sizeof(struct sockaddr_in));
    VLOG(1) << "sendto result: " << n << "(socket: " << socket << " addr: " << addr << " port: " << port << ')';
    if (n < 0)
        LOG(ERROR) << "sendto error";

    char tmmmm[INET6_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &c, tmmmm, INET6_ADDRSTRLEN) == 0) {
        LOG(ERROR) << "plouf";
    } else
        LOG(INFO) << "allo : " << tmmmm;
}
#endif
