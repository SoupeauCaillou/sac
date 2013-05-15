#include <enet/enet.h>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <cstring>
#include <list>
#include <vector>
#include <glm/gtc/random.hpp>

#include "base/Log.h"

#define NICKNAME_PKT 1
#define PORT_PKT 2
struct LobbyPacket {
    uint8_t type, nameLength, server;
    unsigned int remoteIp;
    unsigned short remotePort, localPort;
};

struct MatchMaking {
    ENetPeer* serverModePeer;
    ENetPeer* clientModePeer;
    uint16_t serverPort;
};

static ENetPacket* peerAndNickToPacket(ENetPeer* peer, const std::string& name, unsigned short portA, unsigned short portB, bool server);

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

    std::map<ENetPeer*, std::string> peer2Name;
    std::list<ENetPeer*> peerWaiting;
    std::vector<MatchMaking> inProgress;
    ENetEvent event;
    while (enet_host_service(server, &event, 1000) >= 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_CONNECT: {
                LOGI("Client connection: @" << event.peer->address.host << ":" << event.peer->address.port)
                peer2Name.erase(event.peer);
                break;
            }
            case ENET_EVENT_TYPE_DISCONNECT: {
                LOGI("Client disconnection: @" << event.peer->address.host << ":" << event.peer->address.port)
                peer2Name.erase(event.peer);
                break;
            }
            case ENET_EVENT_TYPE_RECEIVE: {
                ENetPacket * packet = event.packet;
                uint8_t type = packet->data[0];

                if (type == PORT_PKT) {
                    uint16_t port;
                    memcpy(&port, &packet->data[1], sizeof(port));
                    port = ntohs(port);

                    for (unsigned i = 0; i < inProgress.size(); i++) {
                        if (inProgress[i].clientModePeer == event.peer) {
                            LOGI("Received client mode local port (" << port << ")")
                            // create packet for player2
                            ENetPacket* p2 = peerAndNickToPacket(
                                inProgress[i].clientModePeer,
                                peer2Name[inProgress[i].clientModePeer],
                                port,
                                inProgress[i].serverPort,
                                true);
                            enet_peer_send(inProgress[i].serverModePeer, 0, p2);

                            inProgress.erase(inProgress.begin() + i);
                        }
                    }
                } else if (type == NICKNAME_PKT) {
                    peerWaiting.push_back(event.peer);
                    uint8_t length = packet->data[1];
                    char tmp[256];
                    memcpy(tmp, &packet->data[2], length);
                    tmp[length] = 0;
                    LOGI("Client name: '" << tmp << "'")
                    peer2Name[event.peer] = tmp;
                    break;
                } else {
                    LOGW("Ingored packet type : " << (int)type)
                }
                enet_packet_destroy (event.packet);
            }
            case ENET_EVENT_TYPE_NONE: {
				}

        }
        // if we have 2 players with a name : connect them
        if (peerWaiting.size() >= 2) {
            LOGI("Matchmaking in progress !")
            int portA = glm::linearRand(55000.0f, 56000.0f);
            // *** int portB = MathUtil::RandomIntInRange(55000, 56000);
            MatchMaking match;

            std::list<ENetPeer*>::iterator it = peerWaiting.begin();
            match.clientModePeer = *it;
            peerWaiting.erase(it);
            it = peerWaiting.begin();
            match.serverModePeer = *it;
            peerWaiting.erase(it);
            match.serverPort = portA;

            // send packet to client player
            ENetPacket* p = peerAndNickToPacket(match.serverModePeer, peer2Name[match.serverModePeer], match.serverPort, 0, false);

            std::stringstream ss;
            ss << "   -> '" << peer2Name[match.serverModePeer] << "' (S) versus '";

            inProgress.push_back(match);

            ss << peer2Name[match.clientModePeer] << "'";
            LOGI(ss.str());

            LOGI("Send connection info to client")
            enet_peer_send(match.clientModePeer, 0, p);

            enet_host_flush(server);
        }
    }
    return 0;
}

static ENetPacket* peerAndNickToPacket(ENetPeer* peer, const std::string& name, unsigned short portA, unsigned short portB, bool server) {
    uint8_t buffer[1024];
    LobbyPacket* pkt = (LobbyPacket*)&buffer;
    pkt->nameLength = (uint8_t)name.size();
    pkt->remoteIp = peer->address.host;
    pkt->remotePort = portA;
    pkt->localPort = portB;
    pkt->server = server;
    memcpy(&buffer[sizeof(LobbyPacket)], name.c_str(), name.size());
    return enet_packet_create(buffer, sizeof(LobbyPacket) + name.size(), ENET_PACKET_FLAG_RELIABLE);
}
