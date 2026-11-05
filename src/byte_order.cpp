#include "hsms/byte_order.hpp"

namespace hsms {

bool is_little_endian() {
    uint16_t num = 0x0001;
    auto* ptr = reinterpret_cast<uint8_t*>(&num);
    return ptr[0] == 0x01;
}

uint16_t host_to_network16(uint16_t val) {
    if (!is_little_endian()) return val;
    return static_cast<uint16_t>((val << 8) | (val >> 8));
}

uint16_t network_to_host16(uint16_t val) {
    return host_to_network16(val); // Swapping is symmetric
}

uint32_t host_to_network32(uint32_t val) {
    if (!is_little_endian()) return val;
    return ((val & 0x000000FFU) << 24) |
           ((val & 0x0000FF00U) << 8)  |
           ((val & 0x00FF0000U) >> 8)  |
           ((val & 0xFF000000U) >> 24);
}

uint32_t network_to_host32(uint32_t val) {
    return host_to_network32(val); // Swapping is symmetric
}

} // namespace hsms