#include <gtest/gtest.h>
#include "hsms/thread_safe_queue.hpp"
#include "hsms/tcp_server.hpp"

TEST(ThreadSafeQueueTest, PushAndPop) {
    hsms::ThreadSafeQueue<int> queue;
    queue.push(42);
    queue.push(100);

    EXPECT_EQ(queue.size(), 2);
    auto item1 = queue.pop();
    ASSERT_TRUE(item1.has_value());
    EXPECT_EQ(item1.value(), 42);

    auto item2 = queue.pop();
    ASSERT_TRUE(item2.has_value());
    EXPECT_EQ(item2.value(), 100);
}

TEST(TcpServerTest, StartAndStopLifecycle) {
    hsms::TcpServer server(5000);
    EXPECT_TRUE(server.start());
    EXPECT_TRUE(server.is_running());

    server.stop();
    EXPECT_FALSE(server.is_running());
}