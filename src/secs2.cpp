#include "hsms/secs2.hpp"
#include "hsms/byte_order.hpp"
#include <stdexcept>
#include <cstring>

namespace hsms {

SecsItem::SecsItem(std::string ascii_str)
    : format_(ItemFormat::ASCII), value_(std::move(ascii_str)) {}

SecsItem::SecsItem(const char* ascii_str)
    : format_(ItemFormat::ASCII), value_(std::string(ascii_str)) {}

SecsItem::SecsItem(uint8_t u1_val)
    : format_(ItemFormat::U1), value_(u1_val) {}

SecsItem::SecsItem(uint32_t u4_val)
    : format_(ItemFormat::U4), value_(u4_val) {}

SecsItem::SecsItem(int32_t i4_val)
    : format_(ItemFormat::I4), value_(i4_val) {}

SecsItem::SecsItem(bool bool_val)
    : format_(ItemFormat::Boolean), value_(bool_val) {}

SecsItem::SecsItem(SecsItemList list_val)
    : format_(ItemFormat::List), value_(std::move(list_val)) {}

std::vector<uint8_t> SecsItem::serialize() const {
    std::vector<uint8_t> out;

    uint8_t fmt_code = (static_cast<uint8_t>(format_) & 0x3F) << 2;

    std::vector<uint8_t> payload;
    switch (format_) {
        case ItemFormat::ASCII: {
            const auto& str = std::get<std::string>(value_);
            payload.assign(str.begin(), str.end());
            break;
        }
        case ItemFormat::U1: {
            payload.push_back(std::get<uint8_t>(value_));
            break;
        }
        case ItemFormat::U4: {
            uint32_t net_val = host_to_network32(std::get<uint32_t>(value_));
            payload.resize(4);
            std::memcpy(payload.data(), &net_val, 4);
            break;
        }
        case ItemFormat::I4: {
            int32_t raw_val = std::get<int32_t>(value_);
            uint32_t net_val = host_to_network32(static_cast<uint32_t>(raw_val));
            payload.resize(4);
            std::memcpy(payload.data(), &net_val, 4);
            break;
        }
        case ItemFormat::Boolean: {
            payload.push_back(std::get<bool>(value_) ? 0xFF : 0x00);
            break;
        }
        case ItemFormat::List: {
            const auto& list = std::get<SecsItemList>(value_);
            for (const auto& child : list) {
                auto child_bytes = child.serialize();
                payload.insert(payload.end(), child_bytes.begin(), child_bytes.end());
            }
            break;
        }
        default:
            break;
    }

    size_t length = (format_ == ItemFormat::List) 
        ? std::get<SecsItemList>(value_).size() 
        : payload.size();

    // Determine number of length bytes needed (1 byte for lengths < 256)
    uint8_t len_bytes_count = 1;
    if (length > 65535) len_bytes_count = 3;
    else if (length > 255) len_bytes_count = 2;

    uint8_t format_byte = fmt_code | (len_bytes_count & 0x03);
    out.push_back(format_byte);

    if (len_bytes_count == 1) {
        out.push_back(static_cast<uint8_t>(length));
    } else if (len_bytes_count == 2) {
        out.push_back(static_cast<uint8_t>((length >> 8) & 0xFF));
        out.push_back(static_cast<uint8_t>(length & 0xFF));
    } else {
        out.push_back(static_cast<uint8_t>((length >> 16) & 0xFF));
        out.push_back(static_cast<uint8_t>((length >> 8) & 0xFF));
        out.push_back(static_cast<uint8_t>(length & 0xFF));
    }

    out.insert(out.end(), payload.begin(), payload.end());
    return out;
}

std::pair<SecsItem, size_t> SecsItem::deserialize(const uint8_t* data, size_t len) {
    if (len < 2) throw std::runtime_error("Buffer underflow reading SECS-II header");

    uint8_t fmt_byte = data[0];
    uint8_t fmt_code = (fmt_byte >> 2) & 0x3F;
    uint8_t len_count = fmt_byte & 0x03;

    // SEMI E5 §6.2: Length byte count must be 1, 2, or 3 (0 is invalid)
    if (len_count == 0) {
        throw std::runtime_error("Invalid SECS-II length byte count of 0");
    }

    if (len < static_cast<size_t>(1 + len_count)) {
        throw std::runtime_error("Buffer underflow reading length bytes");
    }

    size_t length = 0;
    for (size_t i = 0; i < len_count; ++i) {
        length = (length << 8) | data[1 + i];
    }

    size_t header_len = 1 + len_count;
    auto fmt = static_cast<ItemFormat>(fmt_code);

    if (fmt == ItemFormat::List) {
        SecsItemList children;
        size_t current_offset = header_len;
        for (size_t i = 0; i < length; ++i) {
            auto [child, consumed] = SecsItem::deserialize(data + current_offset, len - current_offset);
            children.push_back(child);
            current_offset += consumed;
        }
        return {SecsItem(children), current_offset};
    }

    if (len < header_len + length) {
        throw std::runtime_error("Buffer underflow reading SECS-II payload");
    }

    const uint8_t* payload = data + header_len;

	switch (fmt) {
		case ItemFormat::ASCII: {
			std::string str(reinterpret_cast<const char*>(payload), length);
			SecsItem item(str);
			return {std::move(item), header_len + length};
		}
		case ItemFormat::U1: {
			SecsItem item(payload[0]);
			return {std::move(item), header_len + 1};
		}
		case ItemFormat::U4: {
			uint32_t raw_val;
			std::memcpy(&raw_val, payload, 4);
			SecsItem item(network_to_host32(raw_val));
			return {std::move(item), header_len + 4};
		}
		case ItemFormat::I4: {
			uint32_t raw_val;
			std::memcpy(&raw_val, payload, 4);
			int32_t host_val = static_cast<int32_t>(network_to_host32(raw_val));
			SecsItem item(host_val);
			return {std::move(item), header_len + 4};
		}
		case ItemFormat::Boolean: {
			SecsItem item(payload[0] != 0x00);
			return {std::move(item), header_len + 1};
		}
		default:
			break;
	}

	SecsItem item;
	return {std::move(item), header_len + length};
}

std::vector<uint8_t> SecsMessage::encode(uint16_t session_id) const {
    std::vector<uint8_t> payload = root_item.serialize();

    Header hdr{};
    hdr.session_id   = session_id;
    hdr.header_byte2 = stream & 0x7F;
    if (wait_bit) hdr.header_byte2 |= 0x80;
    hdr.header_byte3 = function;
    hdr.ptype        = 0;
    hdr.stype        = SType::DataMessage;
    hdr.system_bytes = system_bytes;

    auto hdr_bytes = hdr.serialize();

    // Calculate total frame length (10-byte header + SECS-II payload)
    uint32_t total_length = static_cast<uint32_t>(HSMS_HEADER_SIZE + payload.size());
    uint32_t net_length = host_to_network32(total_length);

    std::vector<uint8_t> frame;
    frame.resize(4 + HSMS_HEADER_SIZE + payload.size());

    std::memcpy(frame.data(), &net_length, 4);
    std::memcpy(frame.data() + 4, hdr_bytes.data(), HSMS_HEADER_SIZE);
    if (!payload.empty()) {
        std::memcpy(frame.data() + 4 + HSMS_HEADER_SIZE, payload.data(), payload.size());
    }

    return frame;
}

SecsMessage SecsMessage::create_s1f1(uint32_t system_bytes) {
    SecsMessage msg;
    msg.stream = 1;
    msg.function = 1;
    msg.wait_bit = true;
    msg.system_bytes = system_bytes;
    msg.root_item = SecsItem(SecsItemList{});
    return msg;
}

SecsMessage SecsMessage::create_s1f2(uint32_t system_bytes, const std::string& model, const std::string& version) {
    SecsMessage msg;
    msg.stream = 1;
    msg.function = 2;
    msg.wait_bit = false;
    msg.system_bytes = system_bytes;
    msg.root_item = SecsItem(SecsItemList{
        SecsItem(model),
        SecsItem(version)
    });
    return msg;
}

SecsMessage SecsMessage::create_s2f41(uint32_t system_bytes, const std::string& command) {
    SecsMessage msg;
    msg.stream = 2;
    msg.function = 41;
    msg.wait_bit = true;
    msg.system_bytes = system_bytes;
    msg.root_item = SecsItem(SecsItemList{
        SecsItem(command),
        SecsItem(SecsItemList{})
    });
    return msg;
}

SecsMessage SecsMessage::create_s6f11(uint32_t system_bytes, uint32_t data_id, uint32_t ceid) {
    SecsMessage msg;
    msg.stream = 6;
    msg.function = 11;
    msg.wait_bit = true;
    msg.system_bytes = system_bytes;
    msg.root_item = SecsItem(SecsItemList{
        SecsItem(data_id),
        SecsItem(ceid),
        SecsItem(SecsItemList{})
    });
    return msg;
}

} // namespace hsms