#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "hsms/secs2.hpp"
#include "hsms/state_machine.hpp"
#include "hsms/thread_safe_queue.hpp"

// Tests handling of malformed SECS-II binary streams (truncated length bytes)
TEST(RobustnessTest, TruncatedSecsBufferHandling) {
    // Malformed buffer: Format byte declares U4 with 1 length byte (fmt=0x2C, len_count=1 -> 0xB1)
    std::vector<uint8_t> malformed_buffer = {
        0xB1, // U4 Format byte (fmt=0x2C, len_count=1)
        0x04, // Declared length = 4 bytes
        0x12, 0x34 // Truncated: missing 2 bytes
    };

    EXPECT_THROW(
        {
            auto res = hsms::SecsItem::deserialize(malformed_buffer.data(), malformed_buffer.size());
            (void)res;
        },
        std::runtime_error
    );
}

// Tests unexpected HSMS control frame handling in NOT SELECTED state
TEST(RobustnessTest, InvalidStateControlMessageHandling) {
    hsms::StateMachine sm;
    sm.on_tcp_connect(); // State is now NotSelected

    // Attempt to send a Data Message (SType = 0) before Selection handshake
    hsms::Header invalid_data_hdr{};
    invalid_data_hdr.session_id   = 1;
    invalid_data_hdr.stype        = hsms::SType::DataMessage;
    invalid_data_hdr.system_bytes = 42;

    hsms::Header rsp{};
    bool processed = sm.process_incoming_header(invalid_data_hdr, rsp);

    // State machine should reject data messages in NotSelected state
    EXPECT_FALSE(processed);
    EXPECT_EQ(sm.get_state(), hsms::State::NotSelected);
}

// Multi-threaded stress test for ThreadSafeQueue under concurrent producer-consumer load
TEST(RobustnessTest, ConcurrentQueueStressTest) {
    hsms::ThreadSafeQueue<int> queue;
    constexpr int ITEM_COUNT = 1000;

    // Producer thread: pushes 1,000 items
    std::thread producer([&queue, ITEM_COUNT]() {
        for (int i = 0; i < ITEM_COUNT; ++i) {
            queue.push(i);
        }
    });

    // Consumer thread: pops 1,000 items
    std::vector<int> popped_items;
    popped_items.reserve(ITEM_COUNT);

    std::thread consumer([&queue, &popped_items, ITEM_COUNT]() {
        for (int i = 0; i < ITEM_COUNT; ++i) {
            auto item = queue.pop();
            if (item.has_value()) {
                popped_items.push_back(item.value());
            }
        }
    });

    producer.join();
    consumer.join();

    EXPECT_EQ(popped_items.size(), ITEM_COUNT);
}