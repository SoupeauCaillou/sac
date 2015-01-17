#include "NetworkAPILinuxPacket.h"

#include "util/Serializer.h"

#if SAC_NETWORK

Packet::Enum LobbyPacket::getPacketType(ENetPacket * packet) {
        LOGF_IF(packet->dataLength < sizeof(Packet::Enum), "Invalid packet size: " << packet->dataLength);
        Packet::Enum type;
        memcpy(&type, packet->data, sizeof(Packet::Enum));
        return type;
}

void LobbyPacket::fromENetPacket(ENetPacket * packet) {
    Serializer s;
    addProperties(s);

    memcpy(&type, packet->data, sizeof(Packet::Enum));
    s.deserializeObject(&packet->data[4], packet->dataLength - 4, this);
}

ENetPacket* LobbyPacket::toENetPacket() {
    Serializer s;
    addProperties(s);
    uint8_t* ptr = new uint8_t[s.size(this) + 4];
    memcpy(ptr, &type, sizeof(Packet::Enum));
    unsigned size = s.serializeObject(&ptr[4], this);

    return enet_packet_create(ptr, size + 4, ENET_PACKET_FLAG_RELIABLE);
}

void LoginPacket::addProperties(Serializer& s) {
    s.add(new StringProperty("name", OFFSET_PTR(name, this)));
}

void RoomIdPacket::addProperties(Serializer& s) {
    s.add(new StringProperty("room_id", OFFSET_PTR(roomId, this)));
}

void JoinRoomPacket::addProperties(Serializer& s) {
    s.add(new StringProperty("room_id", OFFSET_PTR(roomId, this)));
}

void InvitePacket::addProperties(Serializer& s) {
    s.add(new StringProperty("room_id", OFFSET_PTR(roomId, this)));
}

void RoomClosedPacket::addProperties(Serializer& s) {
    s.add(new StringProperty("room_id", OFFSET_PTR(roomId, this)));
}

void EnteringRoomPacket::addProperties(Serializer& s) {
    s.add(new StringProperty("room_id", OFFSET_PTR(roomId, this)));
    s.add(new StringProperty("player_name", OFFSET_PTR(playerName, this)));
}

void ConnectionInfoPacket::addProperties(Serializer& s) {
    s.add(new Property<int>("host", OFFSET_PTR(address.host, this)));
    s.add(new Property<short>("port", OFFSET_PTR(address.port, this)));
}

void PlayersInRoomPacket::addProperties(Serializer& s) {
    s.add(new VectorProperty<std::string>("names", OFFSET_PTR(names, this)));
    s.add(new VectorProperty<int>("states", OFFSET_PTR(states, this)));
}

void GuidPacket::addProperties(Serializer& s) {
        s.add(new Property<int>("guid", OFFSET_PTR(guid, this)));
}
#endif