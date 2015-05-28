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



#include <enet/enet.h>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <cstring>
#include <list>
#include <algorithm>
#include <vector>
#include <glm/gtc/random.hpp>

#include "base/Log.h"
#include "api/linux/NetworkAPILinuxPacket.h"
#include "api/NetworkAPI.h"
#include "api/NetworkAPI.h"
#include "base/TimeUtil.h"
#include "base/Frequency.h"

#include <sys/socket.h>
#include <netdb.h>

/////////////////////// Example packets sequence
// PNL: packet sent by Player N to Lobby
// LPN: packet sent by Lobby to Player N
//
/*
  1. P1L: Login
  2. LP1: AckLogin
  3. P1L: CreateRoom
  4. LP1: RoomId
  5. P1L: ConnectionInfo
  6. P2L: Login
  7. L2P: AckLogin
  8. L2P: Invite
  9. L1P: Entering
 10. P2L: JoinRoom
 11. L2P: ConnectionInfo
 12. P2L: ConnectionInfo
 13. L1P: ConnectionInfo
*/


struct Player {
    Player(const std::string& n, ENetPeer* p) : name(n), peer(p) {}
    std::string name;
    ENetPeer* peer;
};

struct ConnectionDetails {
    ConnectionDetails() {
        address.host = address.port = 0;
    }
    ENetAddress address;
};

struct Room {
    Room(Player* p, const std::string& n) : name(n), creator(p), participantListUpdate(1.0) {}

    std::string name;

    Player* creator;
    std::map<Player*, NetworkStatus::Enum> participants;

    Frequency<float> participantListUpdate;
    ConnectionDetails details;

    bool acceptsParticipants() const {
        return details.address.port != 0;
    }
};


static Player* lookupPlayer(std::vector<Player*> players, ENetPeer* peer);

int main() {
    enet_initialize();

    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 50000;

    ENetHost *server = enet_host_create (&address /* the address to bind the server host to */,
                                 32      /* allow up to 32 clients and/or outgoing connections */,
                                  2      /* allow up to 2 channels to be used, 0 and 1 */,
                                  0      /* assume any amount of incoming bandwidth */,
                                  0      /* assume any amount of outgoing bandwidth */);

    std::vector<Player*> loggedIn;
    std::vector<Room*> rooms;

    // Process network received packets
    ENetEvent event;
    float before = TimeUtil::GetTime();

    while (enet_host_service(server, &event, 1000) >= 0) {
        switch (event.type) {
            // New incoming connection
            case ENET_EVENT_TYPE_CONNECT: {
                LOGI("Client connection: @" << event.peer->address.host << ":" << event.peer->address.port);
                break;
            }
            // Client disconnected
            case ENET_EVENT_TYPE_DISCONNECT: {
                LOGI("Client disconnection: @" << event.peer->address.host << ":" << event.peer->address.port);
                // Destroy associated player
                Player* player = lookupPlayer(loggedIn, event.peer);
                if (player) {
                    for (auto it=rooms.begin(); it!=rooms.end(); ++it) {
                        auto* room = *it;
                        if (room) {
                            if (room->creator == player) {
                                // close room, notify other
                                for (auto& p: room->participants) {
                                    RoomClosedPacket rc(room->name);
                                    enet_peer_send(p.first->peer, 0, rc.toENetPacket());
                                }
                                delete room;
                                *it = 0;
                            }
                        }
                    }
                    std::remove_if(loggedIn.begin(), loggedIn.end(), [player] (Player* p) -> bool { return p == player; });
                    delete player;
                }

                break;
            }
            // Packet received
            case ENET_EVENT_TYPE_RECEIVE: {
                ENetPacket * packet = event.packet;

                LOGI("TYPE: " << LobbyPacket::getPacketType(packet));
                switch (LobbyPacket::getPacketType(packet)) {
                    case Packet::Login: {
                        LoginPacket login;
                        login.fromENetPacket(event.packet);
                        // remember player
                        Player* player = new Player(login.name, event.peer);
                        loggedIn.push_back(player);

                        struct sockaddr_in sin;
                        sin.sin_family = AF_INET;
                        sin.sin_port = ENET_HOST_TO_NET_16 (event.peer->address.port);
                        sin.sin_addr.s_addr = event.peer->address.host;

                        char host[128];
                        getnameinfo((struct sockaddr*)&sin, sizeof(sin),
                            host, sizeof(host),
                            NULL, 0,
                            NI_NUMERICHOST);
                        LOGI("Player '" << login.name << "' logged in from '" << host << "'");

                        // send ack
                        AckLoginPacket ack;
                        enet_peer_send(event.peer, 0, ack.toENetPacket());

                        // if a room exist, send an invite too
                        for (auto* room: rooms) {
                            if (room && room->acceptsParticipants()) {
                                InvitePacket r (room->name);
                                enet_peer_send(event.peer, 0, r.toENetPacket());

                                room->participants.insert(std::make_pair(player, NetworkStatus::JoiningRoom));
                                LOGI("Sent '" << login.name << "' an invite to join room '" << room->name << "'");

                                EnteringRoomPacket enter(room->name, player->name);
                                enet_peer_send(room->creator->peer, 0, enter.toENetPacket());
                            }
                        }

                        break;
                    }
                    case Packet::CreateRoom: {
                        Player* player = lookupPlayer(loggedIn, event.peer);
                        if (!player) {
                            LOGW("Unkown peer from '" << event.peer->address.host << " 'requested room creation");
                        } else {
                            // Create room
                            std::stringstream roomId;
                            roomId << "room_" << rooms.size();
                            RoomIdPacket r (roomId.str());

                            rooms.push_back(new Room(player, roomId.str()));

                            enet_peer_send(event.peer, 0, r.toENetPacket());

                            LOGI("Created room '" << roomId.str() << "' - requested by: '" << player->name << "'");
                        }
                        break;
                    }
                    case Packet::ConnectionInfo: {
                        Player* player = lookupPlayer(loggedIn, event.peer);
                        ConnectionInfoPacket conn;
                        conn.fromENetPacket(event.packet);

                        for (auto* room: rooms) {
                            if (!room) continue;
                            // player is either the creator of a room, or a participant
                            if (room->creator == player) {
                                LOGI("Connection info received from creator of room '" << room->name << "'");
                                // fill room connection details
                                room->details.address.host = event.peer->address.host;
                                room->details.address.port = conn.address.port;
                                char tmp[50];
                                if (enet_address_get_host(&event.peer->address, tmp, 50) == 0 && strncmp(tmp, "localhost", 9) == 0) {
                                    room->details.address.host = 0;
                                }

                                LOGI("room->details.address = " << room->details.address.host << ":" << room->details.address.port);
                                LOGE_IF(!room->acceptsParticipants(), "Room still doesn't accept participant... weird");

                                // Invite every idle players
                                for (auto* p: loggedIn) {
                                    bool idle = true;
                                    for (auto* r: rooms) {
                                        if (!r) continue;
                                        if (r->creator == p) idle = false;
                                        else {
                                            for (auto& q: r->participants) {
                                                if (q.first == p) idle = false;
                                            }
                                        }
                                    }
                                    if (idle) {
                                        InvitePacket r (room->name);
                                        enet_peer_send(p->peer, 0, r.toENetPacket());

                                        room->participants.insert(std::make_pair(p, NetworkStatus::JoiningRoom));
                                        LOGI("Sent '" << p->name << "' an invite to join room '" << room->name << "'");

                                        EnteringRoomPacket enter(room->name, p->name);
                                        enet_peer_send(room->creator->peer, 0, enter.toENetPacket());
                                    }

                                }
                                break;
                            } else {
                                for (auto& p: room->participants) {
                                    if (p.first == player) {
                                        conn.address.host = event.peer->address.host;
                                        // notify room creator to enable him to send nat-punch through packets
                                        enet_peer_send(room->creator->peer, 0, conn.toENetPacket());
                                        break;
                                    }
                                }
                            }
                        }
                        break;
                    }
                    case Packet::JoinRoom: {
                        Player* player = lookupPlayer(loggedIn, event.peer);

                        for (auto* room: rooms) {
                            if (!room) continue;
                            // player is either the creator of a room, or a participant
                            if (room->creator == player) {
                                LOGE("JoinRoom packet received from room creator (" << player->name << ')');
                                break;
                            } else {
                                for (auto& p: room->participants) {
                                    if (p.first == player) {
                                        LOGE("JoinRoom packet received from '" << player->name << "'. Sending room '" << room->name << "' connection details.");
                                        // send player connection info
                                        ConnectionInfoPacket conn(room->details.address);
                                        LOGI(conn.address.host);
                                        enet_peer_send(event.peer, 0, conn.toENetPacket());
                                        break;
                                    }
                                }
                            }
                        }
                        break;
                    }
                    case Packet::EnteringRoom: {
                        EnteringRoomPacket enter;
                        enter.fromENetPacket(event.packet);
                        LOGI("ENTER: '" << enter.roomId << "', '" << enter.playerName << "'");
                        for (auto* room: rooms) {
                            if (!room) continue;
                            if (room->name == enter.roomId) {
                                LOGI("Found room");
                                for (auto& p: room->participants) {
                                    if (p.first->name == enter.playerName) {
                                        LOGI("Found player");
                                        room->participants[p.first] = NetworkStatus::ConnectedToServer;
                                        break;
                                    }
                                }
                                break;
                            }
                        }
                        break;
                    }
                    case Packet::AckLogin:
                    case Packet::RoomId:
                    case Packet::RoomClosed:
                    case Packet::Invitation:
                    case Packet::PlayersInRoom:
                        LOGE("Received invalid message type");
                        break;
                    default:
                        break;
                }
                enet_packet_destroy (event.packet);
            }
            default:
                break;
        }
        enet_host_flush(server);

        float now = TimeUtil::GetTime();
        float dt = now - before;
        before = now;
        for (auto* room: rooms) {
            if (room) {
                if ((room->participantListUpdate.accum += dt) > room->participantListUpdate.value) {
                    room->participantListUpdate.accum = 0;

                    PlayersInRoomPacket pkt;
                    pkt.names.push_back(room->creator->name);
                    pkt.states.push_back(NetworkStatus::InRoomAsMaster);
                    for (auto& p: room->participants) {
                        pkt.names.push_back(p.first->name);
                        pkt.states.push_back(p.second);
                    }
                    for (auto& p: room->participants) {
                        enet_peer_send(p.first->peer, 0, pkt.toENetPacket());
                    }
                    enet_peer_send(room->creator->peer, 0, pkt.toENetPacket());
                }
            }
        }
    }
    return 0;
}

static Player* lookupPlayer(std::vector<Player*> players, ENetPeer* peer) {
    for (auto* p: players) {
        if (p->peer == peer) {
            return p;
        }
    }
    return 0;
}
