#include <gtest/gtest.h>
#include "hsms/byte_order.hpp"
#include "hsms/header.hpp"
#include "hsms/state_machine.hpp"

TEST(ByteOrderTest, SixteenBitConversion) {
    uint16_t original = 0x1234;
    uint16_t net = hsms::host_to_network16(original);
    uint16_t host = hsms::network_to_host16(net);
    EXPECT_EQ(host, original);
}

TEST(ByteOrderTest, ThirtyTwoBitConversion) {
    uint32_t original = 0x12345678;
    uint32_t net = hsms::host_to_network32(original);
    uint32_t host = hsms::network_to_host32(net);
    EXPECT_EQ(host, original);
}

TEST(HeaderTest, SerializeAndDeserialize) {
    hsms::Header original{};
    original.session_id   = 0x0001;
    original.header_byte2 = 0x00;
    original.header_byte3 = 0x00;
    original.ptype        = 0;
    original.stype        = hsms::SType::SelectReq;
    original.system_bytes = 0x00000064;

    auto binary = original.serialize();
    auto deserialized = hsms::Header::deserialize(binary);

    EXPECT_EQ(deserialized.session_id, original.session_id);
    EXPECT_EQ(deserialized.ptype, original.ptype);
    EXPECT_EQ(deserialized.stype, original.stype);
    EXPECT_EQ(deserialized.system_bytes, original.system_bytes);
}

TEST(StateMachineTest, TransitionLifecycle) {
    hsms::StateMachine sm;
    EXPECT_EQ(sm.get_state(), hsms::State::NotConnected);

    sm.on_tcp_connect();
    EXPECT_EQ(sm.get_state(), hsms::State::NotSelected);

    // Send Select.req
    hsms::Header select_req{};
    select_req.session_id   = 1;
    select_req.stype        = hsms::SType::SelectReq;
    select_req.system_bytes = 100;

    hsms::Header select_rsp{};
    bool handled = sm.process_incoming_header(select_req, select_rsp);

    EXPECT_TRUE(handled);
    EXPECT_EQ(sm.get_state(), hsms::State::Selected);
    EXPECT_EQ(select_rsp.stype, hsms::SType::SelectRsp);
    EXPECT_EQ(select_rsp.header_byte3, static_cast<uint8_t>(hsms::SelectStatus::Success));
    EXPECT_EQ(select_rsp.system_bytes, 100);
}