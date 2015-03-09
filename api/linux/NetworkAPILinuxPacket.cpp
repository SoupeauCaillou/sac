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
    s.add(new StringProperty(HASH("name", 0x0), OFFSET_PTR(name, this)));
}

void RoomIdPacket::addProperties(Serializer& s) {
    s.add(new StringProperty(HASH("room_id", 0x0), OFFSET_PTR(roomId, this)));
}

void JoinRoomPacket::addProperties(Serializer& s) {
    s.add(new StringProperty(HASH("room_id", 0x0), OFFSET_PTR(roomId, this)));
}

void InvitePacket::addProperties(Serializer& s) {
    s.add(new StringProperty(HASH("room_id", 0x0), OFFSET_PTR(roomId, this)));
}

void RoomClosedPacket::addProperties(Serializer& s) {
    s.add(new StringProperty(HASH("room_id", 0x0), OFFSET_PTR(roomId, this)));
}

void EnteringRoomPacket::addProperties(Serializer& s) {
    s.add(new StringProperty(HASH("room_id", 0x0), OFFSET_PTR(roomId, this)));
    s.add(new StringProperty(HASH("player_name", 0x0), OFFSET_PTR(playerName, this)));
}

void ConnectionInfoPacket::addProperties(Serializer& s) {
    s.add(new Property<int>(HASH("host", 0x0), OFFSET_PTR(address.host, this)));
    s.add(new Property<short>(HASH("port", 0x0), OFFSET_PTR(address.port, this)));
}

void PlayersInRoomPacket::addProperties(Serializer& s) {
    s.add(new VectorProperty<std::string>(HASH("names", 0x0), OFFSET_PTR(names, this)));
    s.add(new VectorProperty<int>(HASH("states", 0x0), OFFSET_PTR(states, this)));
}

void GuidPacket::addProperties(Serializer& s) {
        s.add(new Property<int>(HASH("guid", 0x0), OFFSET_PTR(guid, this)));
}
#endif
