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

#include <enet/enet.h>
class Serializer;
#include <string>
#include <vector>

// Different package types
namespace Packet {
    enum Enum {
        Login = 0,
        AckLogin,
        CreateRoom,
        RoomId,
        JoinRoom,
        EnteringRoom,
        RoomClosed,
        Invitation,
        ConnectionInfo,
        PlayersInRoom,
    };
}

// Network packet type base class
struct LobbyPacket {
    LobbyPacket(Packet::Enum pType): type(pType) {}
    Packet::Enum type;

    static Packet::Enum getPacketType(ENetPacket * packet);

    void fromENetPacket(ENetPacket * packet);
    ENetPacket* toENetPacket();

    virtual void addProperties(Serializer& s) = 0;
};

struct LoginPacket : LobbyPacket {
    std::string name;
    LoginPacket(const std::string& n = "") : LobbyPacket(Packet::Login), name(n) {}

    void addProperties(Serializer& s);
};

struct AckLoginPacket : LobbyPacket {
    AckLoginPacket() : LobbyPacket(Packet::AckLogin) {}
    void addProperties(Serializer& ) {}
};

struct CreateRoomPacket : LobbyPacket {
    CreateRoomPacket() : LobbyPacket(Packet::CreateRoom) {}
    void addProperties(Serializer& ) {}
};

struct RoomIdPacket : LobbyPacket {
	RoomIdPacket(const std::string& r = "") : LobbyPacket(Packet::RoomId), roomId(r) {}

	std::string roomId;
    void addProperties(Serializer& );
};

struct JoinRoomPacket : LobbyPacket {
    JoinRoomPacket(const std::string& r = "") : LobbyPacket(Packet::JoinRoom), roomId(r) {}

    std::string roomId;
    void addProperties(Serializer& );
};

struct InvitePacket : LobbyPacket {
	std::string roomId;

	InvitePacket(const std::string& r = "") : LobbyPacket(Packet::Invitation), roomId(r) {}

    void addProperties(Serializer& );
};

struct RoomClosedPacket : LobbyPacket {
	std::string roomId;

	RoomClosedPacket(const std::string& r = "") : LobbyPacket(Packet::RoomClosed), roomId(r) {}

    void addProperties(Serializer& );
};

struct EnteringRoomPacket : LobbyPacket {
	std::string roomId;
	std::string playerName;

	EnteringRoomPacket(const std::string& r = "", const std::string& p = "") : LobbyPacket(Packet::EnteringRoom), roomId(r), playerName(p) {}

    void addProperties(Serializer& );
};

struct ConnectionInfoPacket : LobbyPacket {
	ENetAddress address;

	ConnectionInfoPacket() : LobbyPacket(Packet::ConnectionInfo) {}
	ConnectionInfoPacket(const ENetAddress& addr) : LobbyPacket(Packet::ConnectionInfo), address(addr) {}

    void addProperties(Serializer& );
};

struct PlayersInRoomPacket : LobbyPacket {
    std::vector<std::string> names;
    std::vector<int> states;

    PlayersInRoomPacket() : LobbyPacket(Packet::PlayersInRoom) {}

    void addProperties(Serializer& );
};