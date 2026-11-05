#ifndef HSMS_HEADER_HPP
#define HSMS_HEADER_HPP

#include <cstdint>
#include <array>
#include <cstddef>

namespace hsms {

/// SEMI E37 Session Types (SType)
enum class SType : uint8_t {
    DataMessage  = 0,
    SelectReq    = 1,
    SelectRsp    = 2,
    DeselectReq  = 3,
    DeselectRsp  = 4,
    LinktestReq  = 5,
    LinktestRsp  = 6,
    RejectReq    = 7,
    SeparateReq  = 9
};

/// Select Status responses for SelectRsp
enum class SelectStatus : uint8_t {
    Success         = 0,
    AlreadyActive   = 1,
    NotReady        = 2,
    Exhausted       = 3
};

constexpr size_t HSMS_HEADER_SIZE = 10;

/// Represents an HSMS 10-byte header
struct Header {
    uint16_t session_id{0xFFFF};
    uint8_t  header_byte2{0};
    uint8_t  header_byte3{0};
    uint8_t  ptype{0}; // 0 = SECS-II Encoding
    SType    stype{SType::DataMessage};
    uint32_t system_bytes{0};

    /// Serializes struct members into a 10-byte big-endian binary buffer
    [[nodiscard]] std::array<uint8_t, HSMS_HEADER_SIZE> serialize() const;

    /// Deserializes a 10-byte big-endian binary buffer into a Header struct
    [[nodiscard]] static Header deserialize(const std::array<uint8_t, HSMS_HEADER_SIZE>& buffer);
};

} // namespace hsms

#endif // HSMS_HEADER_HPP