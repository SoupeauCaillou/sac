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

        virtual bool amIGameMaster() const = 0;
};
