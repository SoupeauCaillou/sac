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
#include <cstring>
#include <arpa/inet.h>

struct NetworkAPILinuxImpl::NetworkAPILinuxImplDatas {
    NetworkAPILinuxImplDatas() : lobbyClient(0) {}

    std::string lobbyNickName;
    pthread_t lobbyThread;
    std::string lobbyServer;
    ENetHost * lobbyClient;
    ENetPeer *lobbyPeer;

    
};

static ENetPacket* convertPacket(const NetworkPacket& pkt, uint32_t flags);

struct LobbyPacket {
    uint8_t nameLength;
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
    datas->lobbyClient = enet_host_create (NULL /* create a client host */,
        1 /* only allow 1 outgoing connection */,
        2 /* allow up 2 channels to be used, 0 and 1 */,
        57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
        14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);
    if (datas->lobbyClient == 0) {
        LOGE("Failed to create lobby client");
        return;
    }
    ENetAddress address;
    ENetEvent event;
    enet_address_set_host (&address, datas->lobbyServer.c_str());
    address.port = 54321;
    
    LOGI("Trying to connect to loby server: %s:%d", datas->lobbyServer.c_str(), address.port);
    datas->lobbyPeer = enet_host_connect (datas->lobbyClient, &address, 2, 0);
    if (enet_host_service (datas->lobbyClient, &event, 50000) > 0) {
        LOGI("Connection to lobby successful");
    } else {
        LOGI("Connection to lobby failed");
        enet_host_destroy(datas->lobbyClient);
        datas->lobbyClient = 0;
        return;
    }

    // on successfull connect, send nickname
    NetworkPacket nickPkt;
    nickPkt.size = 1 + datas->lobbyNickName.length();
    nickPkt.data = new uint8_t[nickPkt.size];
    nickPkt.data[0] = (uint8_t)datas->lobbyNickName.length();
    memcpy(&nickPkt.data[1], datas->lobbyNickName.c_str(), datas->lobbyNickName.length());
    
    enet_peer_send(datas->lobbyPeer, 0, convertPacket(nickPkt, ENET_PACKET_FLAG_RELIABLE));
    enet_host_flush(datas->lobbyClient);
    
    sendPacket(nickPkt);
    
    bool failure = false;
    while (!failure) {
        int ret = enet_host_service(datas->lobbyClient, &event, 0);
        
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
                    LOGI("Other player: '%s' @ %s:%d. Local port: %d", tmp, inet_ntop(AF_INET, &ad.s_addr, tmp2, INET_ADDRSTRLEN), pkt->remotePort, pkt->localPort);
                    break;
                }
                default:
                    break;
            }
        }
    }
    enet_host_destroy(datas->lobbyClient);
    datas->lobbyClient = 0;
}

void NetworkAPILinuxImpl::connectToLobby(const std::string& nick, const char* addr) {
    if (datas->lobbyClient) {
        return;
    }
    datas->lobbyServer = addr;
    datas->lobbyNickName = nick;
    datas->lobbyNickName.resize(64);

    pthread_create(&datas->lobbyThread, 0, startLobbyThread, this);
}

bool NetworkAPILinuxImpl::isConnectedToAnotherPlayer() {
 
}

NetworkPacket NetworkAPILinuxImpl::pullReceivedPacket() {
 
}

void NetworkAPILinuxImpl::sendPacket(NetworkPacket packet) {
    
}

static ENetPacket* convertPacket(const NetworkPacket& pkt, uint32_t flags) {
    return enet_packet_create(pkt.data, pkt.size, flags);
}