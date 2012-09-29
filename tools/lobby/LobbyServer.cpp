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
#include <enet/enet.h>
#include <string>
#include "base/MathUtil.h"
#include <map>
#include <cstring>

struct LobbyPacket {
    uint8_t nameLength;
    unsigned int remoteIp;
    unsigned short remotePort, localPort;
};

static ENetPacket* peerAndNickToPacket(ENetPeer* peer, const std::string& name, unsigned short portA, unsigned short portB);

int main(int argc, char** argv) {
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 54321;

    ENetHost *server = enet_host_create (&address /* the address to bind the server host to */, 
                                 32      /* allow up to 32 clients and/or outgoing connections */,
                                  2      /* allow up to 2 channels to be used, 0 and 1 */,
                                  0      /* assume any amount of incoming bandwidth */,
                                  0      /* assume any amount of outgoing bandwidth */);

    std::map<ENetPeer*, std::string> peer2Name;
    ENetEvent event;
    while (enet_host_service(server, &event, 1000) >= 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                std::cout << "Client connection: @" << event.peer->address.host << ":" << event.peer->address.port << std::endl;
                peer2Name.erase(event.peer);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                std::cout << "Client disconnection: @" << event.peer->address.host << ":" << event.peer->address.port << std::endl;
                peer2Name.erase(event.peer);
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: {
                ENetPacket * packet = event.packet;
                uint8_t length = packet->data[0];
                char tmp[256];
                memcpy(tmp, &packet->data[1], length);
                tmp[length] = 0;
                std::cout << "Client name: '" << tmp << "'" << std::endl;
                peer2Name[event.peer] = tmp;
                break;
            }
        }
        // if we have 2 players with a name : connect them
        if (peer2Name.size() >= 2) {
            std::cout << "Matchmaking in progress !" << std::endl;
            int portA = MathUtil::RandomIntInRange(55000, 56000);
            int portB = MathUtil::RandomIntInRange(55000, 56000);
            std::map<ENetPeer*, std::string>::iterator it = peer2Name.begin();
            // create packet for player1
            ENetPacket* p1 = peerAndNickToPacket(it->first, it->second, portA, portB);
            std::cout << "   -> '" << it->second << "' versus '";
            ++it;
            // create packet for player2
            ENetPacket* p2 = peerAndNickToPacket(it->first, it->second, portB, portA);
            std::cout << it->second << "'" << std::endl;
            
            enet_peer_send(it->first, 0, p1);
            enet_peer_send(peer2Name.begin()->first, 0, p2);
            
            peer2Name.erase(peer2Name.begin());
            peer2Name.erase(peer2Name.begin());
            
            enet_host_flush(server);
        }
    }
    return 0;
}

static ENetPacket* peerAndNickToPacket(ENetPeer* peer, const std::string& name, unsigned short portA, unsigned short portB) {
    uint8_t buffer[1024];
    LobbyPacket* pkt = (LobbyPacket*)&buffer;
    pkt->nameLength = (uint8_t)name.size();
    pkt->remoteIp = peer->address.host;
    pkt->remotePort = portA;
    pkt->localPort = portB;
    memcpy(&buffer[sizeof(LobbyPacket)], name.c_str(), name.size());
    return enet_packet_create(buffer, sizeof(LobbyPacket) + name.size(), ENET_PACKET_FLAG_RELIABLE);
}
