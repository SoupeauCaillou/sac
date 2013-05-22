#pragma once
#include <stdint.h>
#include <string>

struct NetworkPacket {
    int size;
    uint8_t* data;
};

namespace NetworkStatus {
    enum Enum {
        None,
        ConnectingToLobby,
        ConnectedToLobby,
        ConnectionToLobbyFailed,
        ConnectingToServer,
        ConnectedToServer,
        ConnectionToServerFailed
    };
}

class NetworkAPI {
    public:
        virtual void connectToLobby(const std::string& nickName, const char* addr) = 0;
        virtual bool isConnectedToAnotherPlayer() = 0;

        virtual NetworkStatus::Enum getStatus() const = 0;

        virtual NetworkPacket pullReceivedPacket() = 0;
        virtual void sendPacket(NetworkPacket packet) = 0;

        virtual bool amIGameMaster() = 0;
};
