#include <gtest/gtest.h>
#include "hsms/secs2.hpp"

TEST(Secs2ItemTest, AsciiCodec) {
    std::string test_str = "SECS_EMULATOR_DEVICE_01";
    hsms::SecsItem original(test_str);

    auto bytes = original.serialize();
    auto [decoded, consumed] = hsms::SecsItem::deserialize(bytes.data(), bytes.size());

    EXPECT_EQ(consumed, bytes.size());
    EXPECT_EQ(decoded.get_format(), hsms::ItemFormat::ASCII);
    EXPECT_EQ(std::get<std::string>(decoded.get_value()), test_str);
}

TEST(Secs2ItemTest, UnsignedInt4Codec) {
    uint32_t val = 0x87654321;
    hsms::SecsItem original(val);

    auto bytes = original.serialize();
    auto [decoded, consumed] = hsms::SecsItem::deserialize(bytes.data(), bytes.size());

    EXPECT_EQ(consumed, bytes.size());
    EXPECT_EQ(decoded.get_format(), hsms::ItemFormat::U4);
    EXPECT_EQ(std::get<uint32_t>(decoded.get_value()), val);
}

TEST(Secs2ItemTest, NestedListCodec) {
    hsms::SecsItemList list;
    list.emplace_back(std::string("FAB_01"));
    list.emplace_back(static_cast<uint32_t>(1001));

    hsms::SecsItem root(list);
    auto bytes = root.serialize();

    auto [decoded, consumed] = hsms::SecsItem::deserialize(bytes.data(), bytes.size());
    EXPECT_EQ(decoded.get_format(), hsms::ItemFormat::List);

    const auto& decoded_list = std::get<hsms::SecsItemList>(decoded.get_value());
    ASSERT_EQ(decoded_list.size(), 2);
    EXPECT_EQ(std::get<std::string>(decoded_list[0].get_value()), "FAB_01");
    EXPECT_EQ(std::get<uint32_t>(decoded_list[1].get_value()), 1001);
}

TEST(Secs2MessageTest, EncodeS1F2) {
    auto msg = hsms::SecsMessage::create_s1f2(500, "MODEL_AX1", "v2.4.0");
    auto encoded_frame = msg.encode(1);

    // Frame size = 4 bytes length + 10 bytes header + payload
    EXPECT_GT(encoded_frame.size(), 14);

    // Header byte 2 (Stream) = 1, Header byte 3 (Function) = 2
    EXPECT_EQ(encoded_frame[4 + 2], 1);
    EXPECT_EQ(encoded_frame[4 + 3], 2);
}