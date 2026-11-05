#include <gtest/gtest.h>
#include "hsms/common.hpp"

// Verifies core library header and implementation linkage
TEST(SanityCheck, LibraryVersionTest) {
    EXPECT_EQ(hsms::get_version(), "SECS/HSMS Emulator v1.0.0");
}

// Verifies basic assertion execution
TEST(SanityCheck, BasicAssertion) {
    ASSERT_TRUE(1 + 1 == 2);
}