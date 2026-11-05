#include "hsms/header.hpp"
#include "hsms/byte_order.hpp"

namespace hsms {

std::array<uint8_t, HSMS_HEADER_SIZE> Header::serialize() const {
    std::array<uint8_t, HSMS_HEADER_SIZE> buf{};

    uint16_t net_session = host_to_network16(session_id);
    buf[0] = static_cast<uint8_t>((net_session >> 8) & 0xFF);
    buf[1] = static_cast<uint8_t>(net_session & 0xFF);

    buf[2] = header_byte2;
    buf[3] = header_byte3;
    buf[4] = ptype;
    buf[5] = static_cast<uint8_t>(stype);

    uint32_t net_system = host_to_network32(system_bytes);
    buf[6] = static_cast<uint8_t>((net_system >> 24) & 0xFF);
    buf[7] = static_cast<uint8_t>((net_system >> 16) & 0xFF);
    buf[8] = static_cast<uint8_t>((net_system >> 8) & 0xFF);
    buf[9] = static_cast<uint8_t>(net_system & 0xFF);

    return buf;
}

Header Header::deserialize(const std::array<uint8_t, HSMS_HEADER_SIZE>& buffer) {
    Header hdr{};

    uint16_t raw_session = (static_cast<uint16_t>(buffer[0]) << 8) | buffer[1];
    hdr.session_id = network_to_host16(raw_session);

    hdr.header_byte2 = buffer[2];
    hdr.header_byte3 = buffer[3];
    hdr.ptype        = buffer[4];
    hdr.stype        = static_cast<SType>(buffer[5]);

    uint32_t raw_system = (static_cast<uint32_t>(buffer[6]) << 24) |
                          (static_cast<uint32_t>(buffer[7]) << 16) |
                          (static_cast<uint32_t>(buffer[8]) << 8)  |
                          static_cast<uint32_t>(buffer[9]);
    hdr.system_bytes = network_to_host32(raw_system);

    return hdr;
}

} // namespace hsms