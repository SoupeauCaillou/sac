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
#include "NetworkAPILinuxImpl.h"
#include <enet/enet.h>
#include "../../base/Log.h"
#include "../../base/TimeUtil.h"
#include <cstring>
#include <arpa/inet.h>

struct NetworkAPILinuxImpl::NetworkAPILinuxImplDatas {
    NetworkAPILinuxImplDatas() {
        lobby.client = 0;
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
        bool connected;
    } match;

    
};

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
}

static void* startLobbyThread(void* p) {
    NetworkAPILinuxImpl* ptr = static_cast<NetworkAPILinuxImpl*> (p);
    ptr->runLobbyThread();
    return 0;
}

void NetworkAPILinuxImpl::runLobbyThread() {
    datas->match.connected = false;
    datas->lobby.client = enet_host_create (NULL /* create a client host */,
        1 /* only allow 1 outgoing connection */,
        2 /* allow up 2 channels to be used, 0 and 1 */,
        57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
        14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);
    if (datas->lobby.client == 0) {
        LOGE("Failed to create lobby client");
        return;
    }
    ENetAddress address;
    ENetEvent event;
    enet_address_set_host (&address, datas->lobby.server.c_str());
    address.port = 54321;
    
    LOGI("Trying to connect to loby server: %s:%d", datas->lobby.server.c_str(), address.port);
    datas->lobby.peer = enet_host_connect (datas->lobby.client, &address, 2, 0);
    if (enet_host_service (datas->lobby.client, &event, 50000) > 0) {
        LOGI("Connection to lobby successful");
    } else {
        LOGI("Connection to lobby failed");
        enet_host_destroy(datas->lobby.client);
        datas->lobby.client = 0;
        return;
    }

    // on successfull connect, send nickname
    
    
    enet_peer_send(datas->lobby.peer, 0, convertPacket(createNickNamePacket(datas->lobby.nickName), ENET_PACKET_FLAG_RELIABLE));
    enet_host_flush(datas->lobby.client);
    
    bool failure = false;
    while (!failure) {
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
                    LOGI("Other player: '%s' @ %s:%d. Local port: %d (server: %d)", tmp, inet_ntop(AF_INET, &ad.s_addr, tmp2, INET_ADDRSTRLEN), pkt->remotePort, pkt->localPort, pkt->server);

                    if (pkt->server) {
                        datas->match.connected = connectToOtherPlayerServerMode(tmp2, pkt->remotePort, pkt->localPort);
                        break;
                    } else {
                        datas->match.connected = connectToOtherPlayerClientMode(tmp2, pkt->remotePort);
                        break;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }
    enet_host_destroy(datas->lobby.client);
    datas->lobby.client = 0;
}

bool NetworkAPILinuxImpl::connectToOtherPlayerServerMode(const char* addr, uint16_t remotePort, uint16_t localPort) {
    ENetAddress address;
    LOGI("Creating server socket (%d)", localPort);
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
        int ret = enet_host_service(datas->match.host, &event, 100);
        if (ret < 0)
            return false;
        switch(event.type) {
            /*case ENET_EVENT_TYPE_NONE: {
                // send punch-through UDP packet
                // ...
                break;
            }*/
            case ENET_EVENT_TYPE_CONNECT: {
                datas->match.peer = event.peer;
                LOGI("Received connection");
                return true;
            }
            default:
                break;
        }
    }
    enet_host_destroy(datas->match.host);
    datas->match.host = 0;
    return false;
}
#include <iostream>
bool NetworkAPILinuxImpl::connectToOtherPlayerClientMode(const char* addr, uint16_t remotePort) {
    datas->match.host = enet_host_create (0,
                                  1      /* allow up to 32 clients and/or outgoing connections */,
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
    while ((TimeUtil::getTime() - begin)< 5) {
        int ret = enet_host_service(datas->match.host, &event, 100);
        if (ret < 0)
            return false;
        switch(event.type) {
            case ENET_EVENT_TYPE_NONE:
                break;
            case ENET_EVENT_TYPE_CONNECT:
                datas->match.peer = event.peer;
                return true;
            default:
                break;
        }
        
        if (localPort == 0) {
            struct sockaddr_in sin;
            socklen_t addrlen = sizeof(sin);
            int ret = getsockname(datas->match.host->socket, (struct sockaddr *)&sin, &addrlen);
            if( ret == 0 && sin.sin_family == AF_INET && addrlen == sizeof(sin)) {
                localPort = ntohs(sin.sin_port);
                LOGI("Local port: %d", localPort);
                enet_peer_send(datas->lobby.peer, 0, convertPacket(createLocalPortPacket(localPort), ENET_PACKET_FLAG_RELIABLE));
                enet_host_flush(datas->lobby.client);
            }
        }
    }
    return false;
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
 
}

void NetworkAPILinuxImpl::sendPacket(NetworkPacket packet) {
    
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
    memcpy(&nickPkt.data[1], nickName.c_str(), nickName.length());
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