#ifndef HSMS_SECS2_HPP
#define HSMS_SECS2_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include <memory>
#include "hsms/header.hpp"

namespace hsms {

/// SEMI E5 Format Codes (Upper 6 bits of format byte)
enum class ItemFormat : uint8_t {
    List    = 0x00, // 000000
    Binary  = 0x08, // 010000
    Boolean = 0x09, // 010001
    ASCII   = 0x10, // 010000
    I4      = 0x1C, // 011100
    U1      = 0x29, // 101001
    U4      = 0x2C  // 101100
};

class SecsItem;
using SecsItemList = std::vector<SecsItem>;

/// Represents a single dynamically-typed SECS-II Data Item
class SecsItem {
public:
    using ValueType = std::variant<
        std::monostate,
        std::string,
        uint8_t,
        uint32_t,
        int32_t,
        bool,
        std::vector<uint8_t>,
        SecsItemList
    >;

    SecsItem() = default;

    // Constructors for supported data types
    explicit SecsItem(std::string ascii_str);
	explicit SecsItem(const char* ascii_str);
    explicit SecsItem(uint8_t u1_val);
    explicit SecsItem(uint32_t u4_val);
    explicit SecsItem(int32_t i4_val);
    explicit SecsItem(bool bool_val);
    explicit SecsItem(SecsItemList list_val);

    [[nodiscard]] ItemFormat get_format() const noexcept { return format_; }
    [[nodiscard]] const ValueType& get_value() const noexcept { return value_; }

    /// Encodes item into raw SECS-II binary payload bytes
    [[nodiscard]] std::vector<uint8_t> serialize() const;

    /// Decodes raw SECS-II binary bytes into a SecsItem (returns item and consumed byte count)
    [[nodiscard]] static std::pair<SecsItem, size_t> deserialize(const uint8_t* data, size_t len);

private:
    ItemFormat format_{ItemFormat::List};
    ValueType value_{std::monostate{}};
};

/// High-level SECS-II Message Container
struct SecsMessage {
    uint8_t stream{0};
    uint8_t function{0};
    bool wait_bit{false};
    uint32_t system_bytes{0};
    SecsItem root_item{};

    /// Serializes entire SECS-II message (Header + Formatted Payload)
    [[nodiscard]] std::vector<uint8_t> encode(uint16_t session_id) const;

    /// Helper generator for S1F1 "Are You There?"
    [[nodiscard]] static SecsMessage create_s1f1(uint32_t system_bytes);

    /// Helper generator for S1F2 "On Line Data" Response
    [[nodiscard]] static SecsMessage create_s1f2(uint32_t system_bytes, const std::string& model, const std::string& version);

    /// Helper generator for S2F41 "Host Command Send" (e.g. START/STOP)
    [[nodiscard]] static SecsMessage create_s2f41(uint32_t system_bytes, const std::string& command);

    /// Helper generator for S6F11 "Event Report Send"
    [[nodiscard]] static SecsMessage create_s6f11(uint32_t system_bytes, uint32_t data_id, uint32_t ceid);
};

} // namespace hsms

#endif // HSMS_SECS2_HPP