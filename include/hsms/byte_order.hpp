#ifndef HSMS_BYTE_ORDER_HPP
#define HSMS_BYTE_ORDER_HPP

#include <cstdint>

namespace hsms {

/// Determines if host architecture is little-endian at runtime
[[nodiscard]] bool is_little_endian();

/// Converts 16-bit integer from host byte order to big-endian (network byte order)
[[nodiscard]] uint16_t host_to_network16(uint16_t val);

/// Converts 16-bit integer from big-endian (network byte order) to host byte order
[[nodiscard]] uint16_t network_to_host16(uint16_t val);

/// Converts 32-bit integer from host byte order to big-endian (network byte order)
[[nodiscard]] uint32_t host_to_network32(uint32_t val);

/// Converts 32-bit integer from big-endian (network byte order) to host byte order
[[nodiscard]] uint32_t network_to_host32(uint32_t val);

} // namespace hsms

#endif // HSMS_BYTE_ORDER_HPP