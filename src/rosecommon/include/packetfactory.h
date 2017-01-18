#pragma once

#include "singleton.h"
#include "epackettype.h"
#include "crosepacket.h"
#include <unordered_map>
#include <functional>

namespace RoseCommon {

class PacketFactory : public Singleton<PacketFactory> {
    public:
        template <ePacketType Type, class Class>
        void registerPacket() {
            mapping_[to_underlying(Type)] = [](uint8_t buffer[MAX_PACKET_SIZE]) -> std::unique_ptr<CRosePacket> {
                return std::unique_ptr<CRosePacket>(new Class(buffer));
            };
        }

        std::unique_ptr<CRosePacket> getPacket(uint8_t buffer[MAX_PACKET_SIZE]) {
            try {
                return mapping_.at(to_underlying(CRosePacket::type(buffer)))(buffer);
            } catch (std::out_of_range) {
                return nullptr;
            }
        }

    private:
        using Wrapper = std::function<std::unique_ptr<CRosePacket>(uint8_t[MAX_PACKET_SIZE])>;

        std::unordered_map<std::underlying_type_t<ePacketType>, Wrapper> mapping_;
};

template <ePacketType Type, class Class>
bool registerPacket() {
    PacketFactory::getInstance().registerPacket<Type, Class>();
    return true;
}

template <ePacketType Type>
struct find_send_class;

template <ePacketType Type>
using find_send_class_t = typename find_send_class<Type>::type;

template <ePacketType Type>
struct find_recv_class;

template <ePacketType Type>
using find_recv_class_t = typename find_recv_class<Type>::type;

#define REGISTER_SEND_PACKET(Type, Class) class Class; template <> struct find_send_class<Type> { typedef Class type; };

#define REGISTER_RECV_PACKET(Type, Class) class Class; template <> struct find_recv_class<Type> { typedef Class type; }; static const bool __##Class = registerPacket<Type, Class>();

inline std::unique_ptr<CRosePacket> fetchPacket(uint8_t buffer[MAX_PACKET_SIZE]) {
    return PacketFactory::getInstance().getPacket(buffer);
}

template <ePacketType T>
std::unique_ptr<find_recv_class_t<T>> getPacket(uint8_t buffer[MAX_PACKET_SIZE]) {
    return std::make_unique<find_recv_class_t<T>>(buffer);
}

template <ePacketType T, typename... Args>
std::unique_ptr<find_send_class_t<T>> makePacket(Args&&... args) {
    return std::make_unique<find_send_class_t<T>>(std::forward<Args>(args)...);
}

}