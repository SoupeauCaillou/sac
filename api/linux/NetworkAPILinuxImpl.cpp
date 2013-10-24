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



#include "NetworkAPILinuxImpl.h"

#ifdef SAC_NETWORK
#include <base/TimeUtil.h>
#include <base/Log.h>

#include <enet/enet.h>
#include <arpa/inet.h>

#include <thread>
#include <mutex>
#include <cstring>
#include <iostream>
#include <glm/gtc/random.hpp>
#include "NetworkAPILinuxPacket.h"

class NetworkAPILinuxImpl::NetworkAPILinuxImplDatas {
public:
    NetworkAPILinuxImplDatas() {
        lobby.client = match.host = 0;
        lobby.peer = match.peer = 0;
        match.connected = match.masterMode = false;
        status = NetworkStatus::None;
    }

    // Network datas
    struct {
        std::string nickName;
        std::thread thread;
        std::string server;
        ENetHost * client;
        ENetPeer *peer;
    } lobby;

    struct {
        ENetHost* host;
        ENetPeer* peer;
        bool connected, masterMode;
    } match;

    std::string roomId;

    void setStatus(NetworkStatus::Enum newStatus) {
        std::unique_lock<std::mutex> lock(mutex);
        status = newStatus;
    }
    NetworkStatus::Enum getStatus() {
        std::unique_lock<std::mutex> lock(mutex);
        return status;
    }

    std::vector<std::string> pendingInvites;

    std::string acceptedInvite;
private:
    NetworkStatus::Enum status;
    std::mutex mutex;

    // debug UI
    // Entity background, playButton;
};

static void sendNatPunchThroughPacket(int socket, const char* addr, uint16_t port);

NetworkAPILinuxImpl::NetworkAPILinuxImpl() {
    datas = new NetworkAPILinuxImplDatas();
}

void NetworkAPILinuxImpl::init() {
    enet_initialize();
}

void NetworkAPILinuxImpl::login(const std::string& nickName) {
    LOGF_IF(datas->getStatus() != NetworkStatus::None && datas->getStatus() != NetworkStatus::ConnectionToLobbyFailed,
        "Invalid call to login, state = " << datas->getStatus());

    datas->lobby.server = "127.0.0.1";
    datas->lobby.nickName = nickName;

    datas->lobby.thread = std::thread([this] () -> void {
        runLobbyThread();
    });
}

void NetworkAPILinuxImpl::createRoom() {
    LOGF_IF(datas->getStatus() != NetworkStatus::Logged, "Invalid state for createRoom call");
    datas->setStatus(NetworkStatus::CreatingRoom);
    CreateRoomPacket create;
    enet_peer_send(datas->lobby.peer, 0, create.toENetPacket());
}

NetworkStatus::Enum NetworkAPILinuxImpl::getStatus() const {
    return datas->getStatus();
}

void NetworkAPILinuxImpl::runLobbyThread() {
    datas->setStatus(NetworkStatus::ConnectingToLobby);

    datas->match.connected = datas->match.masterMode = false;
    datas->lobby.client = enet_host_create (NULL /* create a client host */,
        1 /* only allow 1 outgoing connection */,
        2 /* allow up 2 channels to be used, 0 and 1 */,
        57600 / 8 /* 56K modem with 56 Kbps downstream bandwidth */,
        14400 / 8 /* 56K modem with 14 Kbps upstream bandwidth */);
    if (datas->lobby.client == 0) {
        LOGF("Failed to create lobby client");
        datas->setStatus(NetworkStatus::ConnectionToLobbyFailed);
        return;
    }
    ENetAddress address;
    ENetEvent event;
    enet_address_set_host (&address, datas->lobby.server.c_str());
    address.port = 50000;
    datas->setStatus(NetworkStatus::ConnectingToLobby);

    LOGI("Trying to connect to lobby server: " << datas->lobby.server << ':' << address.port);
    datas->lobby.peer = enet_host_connect (datas->lobby.client, &address, 2, 0);

    if (enet_host_service (datas->lobby.client, &event, address.port) > 0) {
        LOGI("Connection to lobby successful");
        datas->setStatus(NetworkStatus::ConnectedToLobby);
    } else {
        LOGI("Connection to lobby failed");
        enet_host_destroy(datas->lobby.client);
        datas->lobby.client = 0;
        datas->setStatus(NetworkStatus::ConnectionToLobbyFailed);
        return;
    }

    // Send Login
    LoginPacket login(datas->lobby.nickName);
    datas->setStatus(NetworkStatus::LoginInProgress);
    enet_peer_send(datas->lobby.peer, 0, login.toENetPacket());

    // Process incoming messages from lobby
    while (true) {
        int ret = enet_host_service(datas->lobby.client, &event, (datas->match.host ? 500 : 1000));

        if (ret < 0) {
            LOGI("Error communicating with lobby, caramba!");
            continue;
        }
        if (ret > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT: {
                    break;
                }
                case ENET_EVENT_TYPE_DISCONNECT: {
                    LOGI("Client disconnection: @" << event.peer->address.host << ":" << event.peer->address.port);
                    break;
                }
                // Packet received
                case ENET_EVENT_TYPE_RECEIVE: {
                    ENetPacket * packet = event.packet;

                    LOGI("Received packet. Type: " << LobbyPacket::getPacketType(packet));
                    switch (LobbyPacket::getPacketType(packet)) {
                        case Packet::AckLogin: {
                            datas->setStatus(NetworkStatus::Logged);
                            break;
                        }
                        case Packet::RoomId: {
                            RoomIdPacket room;
                            room.fromENetPacket(event.packet);
                            datas->roomId = room.roomId;
                            datas->setStatus(NetworkStatus::InRoomAsMaster);

                            // setup server
                            ENetAddress address;
                            uint16_t localPort = (uint16_t) glm::linearRand(2000.0f, 60000.0f);
                            LOGI("Creating server socket (" << localPort << ')');
                            address.host = ENET_HOST_ANY;
                            address.port = localPort;
                            datas->match.host = enet_host_create (&address, 32, 2, 0, 0);
                            datas->match.masterMode = true;

                            // send connection info
                            ConnectionInfoPacket conn;
                            conn.address = address;
                            enet_peer_send(datas->lobby.peer, 0, conn.toENetPacket());
                            break;
                        }
                        case Packet::Invitation: {
                            InvitePacket invite;
                            invite.fromENetPacket(event.packet);
                            if (datas->getStatus() == NetworkStatus::Logged) {
                                datas->pendingInvites.push_back(invite.roomId);
                            } else {
                                LOGW("Ignore invite to room '" << invite.roomId << "', wrong state: " << datas->getStatus());
                            }
                            break;
                        }
                        case Packet::ConnectionInfo: {
                            ConnectionInfoPacket conn;
                            conn.fromENetPacket(event.packet);
                            switch(datas->getStatus()) {
                                case NetworkStatus::InRoomAsMaster: {
                                    // send punch through nat-packet
                                    // sendNatPunchThroughPacket(datas->match.host->socket, addr, remotePort);
                                    LOGW("Send Nat punch through packets");
                                    break;
                                }
                                case NetworkStatus::JoiningRoom: {
                                    datas->match.host = enet_host_create (0, 32, 2, 0, 0);
                                    datas->match.peer = enet_host_connect (datas->match.host, &conn.address, 2, 0);
                                    break;
                                }
                                default: {
                                    LOGW("Ignore connectionInfo packet, wrong state: " << datas->getStatus());
                                    break;
                                }
                            }
                            break;
                        }
                    }
                    enet_packet_destroy (event.packet);
                }
            }
        }
        enet_host_flush(datas->lobby.client);

        if (datas->acceptedInvite != "") {
            datas->setStatus(NetworkStatus::JoiningRoom);
            JoinRoomPacket join(datas->acceptedInvite);
            enet_peer_send(datas->lobby.peer, 0, join.toENetPacket());
        }

        if (datas->match.host && !(datas->match.connected || datas->match.masterMode)) {
            int ret = enet_host_service(datas->match.host, &event, 0);
            if (ret < 0) {
                LOGE("enet_host_service(datas->match.host) failure");
            }
            if (ret > 0) {
                switch (event.type) {
                    case ENET_EVENT_TYPE_CONNECT: {
                        if (!datas->match.masterMode) {
                            datas->setStatus(NetworkStatus::ConnectedToServer);
                            datas->match.connected = true;
                        }
                        else
                            LOGI("New incoming connection");
                        break;
                    }
                    case ENET_EVENT_TYPE_DISCONNECT: {
                        LOGI("Client disconnection: @" << event.peer->address.host << ":" << event.peer->address.port);
                        break;
                    }
                    // Packet received
                    case ENET_EVENT_TYPE_RECEIVE: {
                        enet_packet_destroy (event.packet);
                    }
                }
            }
            enet_host_flush(datas->match.host);
        }
    }
}

unsigned NetworkAPILinuxImpl::getPendingInvitationCount() const {
    return datas->pendingInvites.size();
}

void NetworkAPILinuxImpl::acceptInvitation() {
    datas->acceptedInvite = datas->pendingInvites.back();
    datas->pendingInvites.clear();
}

#if 0
bool NetworkAPILinuxImpl::connectToOtherPlayerServerMode(const char* addr, uint16_t remotePort, uint16_t localPort) {
    ENetAddress address;
    LOGI("Creating server socket (" << localPort << ')');
    address.host = ENET_HOST_ANY;
    address.port = localPort;
    datas->match.host = enet_host_create (&address /* the address to bind the server host to */,
                                  32      /* allow up to 32 clients and/or outgoing connections */,
                                  2      /* allow up to 2 channels to be used, 0 and 1 */,
                                  0      /* assume any amount of incoming bandwidth */,
                                  0      /* assume any amount of outgoing bandwidth */);
    float begin = TimeUtil::GetTime();
    ENetEvent event;
    while ((TimeUtil::GetTime()-begin) < 5) {
        int ret = enet_host_service(datas->match.host, &event, 250);
        if (ret < 0)
            return false;
        switch(event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                datas->match.peer = event.peer;
                LOGI("Received connection");
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
    LOGE("Fail");
    return false;
}

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




    float begin = TimeUtil::GetTime();
    ENetEvent event;
    while ((TimeUtil::GetTime() - begin)< 15) {
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
                LOGI("Local port: " << localPort);
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

    datas->lobby.thread = std::thread([this] () -> void {
        runLobbyThread();
    });
}

bool NetworkAPILinuxImpl::isConnectedToAnotherPlayer() {
    return datas->match.connected;
}
#endif

NetworkPacket NetworkAPILinuxImpl::pullReceivedPacket() {
    NetworkPacket result;
    result.size = 0;
    if (datas->match.host == 0)
        return result;
    ENetEvent event;
    int ret = enet_host_service(datas->match.host, &event, 0);
    if (ret < 0) {
        LOGE("enet_host_service error");
    } else if (ret == 0) {
        // nothing received
    } else {
        switch (event.type) {
            case ENET_EVENT_TYPE_DISCONNECT:
                // probably some cleanup is needed here
                LOGW("Disconnect even received");
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

static ENetPacket* convertPacket(const NetworkPacket& pkt, uint32_t flags) {
    return enet_packet_create(pkt.data, pkt.size, flags);
}

void NetworkAPILinuxImpl::sendPacket(NetworkPacket packet) {
    enet_peer_send(datas->match.peer, 0, convertPacket(packet, 0));
}

static void sendNatPunchThroughPacket(int socket, const char* addr, uint16_t port) {
    // send NAT punch through packet
    char tmp[4];
    struct sockaddr_in c;
    c.sin_port = htons(port);
    c.sin_family = AF_INET;

    if (inet_pton(AF_INET, addr, &c.sin_addr) != 1)
        LOGE("inet_pton");

    int n = sendto(socket, tmp, 4, 0, (struct sockaddr*)&c, sizeof(struct sockaddr_in));
    LOGV(1, "sendto result: " << n << "(socket: " << socket << " addr: " << addr << " port: " << port << ')');
    if (n < 0)
        LOGE("sendto error");

    char tmmmm[INET6_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &c, tmmmm, INET6_ADDRSTRLEN) == 0) {
        LOGE("plouf");
    } else
        LOGI("allo : " << addr << " failed");
}

#else

NetworkAPILinuxImpl::NetworkAPILinuxImpl() {}
void NetworkAPILinuxImpl::runLobbyThread() {}
bool NetworkAPILinuxImpl::connectToOtherPlayerServerMode(const char*, uint16_t, uint16_t) { return false; }
void NetworkAPILinuxImpl::sendPacket(NetworkPacket) {}
bool NetworkAPILinuxImpl::amIGameMaster() const { return false; }
void NetworkAPILinuxImpl::connectToLobby(const std::string&, const char*) { }
bool NetworkAPILinuxImpl::isConnectedToAnotherPlayer() { return false; }
NetworkPacket NetworkAPILinuxImpl::pullReceivedPacket() { return NetworkPacket();}

#endif
