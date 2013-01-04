#pragma once

#include "../NetworkAPI.h"

class NetworkAPILinuxImpl : public NetworkAPI {
    public:
        NetworkAPILinuxImpl();

        void connectToLobby(const std::string& nickName, const char* addr);
        bool isConnectedToAnotherPlayer();

        NetworkPacket pullReceivedPacket();
        void sendPacket(NetworkPacket packet);

        bool amIGameMaster();

        void runLobbyThread();
    private:
        struct NetworkAPILinuxImplDatas;
        NetworkAPILinuxImplDatas* datas;

        bool connectToOtherPlayerServerMode(const char* addr, uint16_t remotePort, uint16_t localPort);
        bool connectToOtherPlayerClientMode(const char* addr, uint16_t remotePort);
};
