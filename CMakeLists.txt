cmake_minimum_required(VERSION 3.15)
project(SECS_HSMS_Emulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Option to enable AddressSanitizer (OFF for local MSVC builds; ON in Linux CI)
option(ENABLE_SANITIZERS "Enable AddressSanitizer" OFF)

if(MSVC)
    add_compile_options(/W4 /WX)
    if(ENABLE_SANITIZERS)
        add_compile_options(/fsanitize=address)
    endif()
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    if(ENABLE_SANITIZERS)
        add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
        add_link_options(-fsanitize=address)
    endif()
endif()

add_library(hsms_core
    src/common.cpp
    src/byte_order.cpp
    src/header.cpp
    src/state_machine.cpp
    src/secs2.cpp
    src/tcp_server.cpp
)

target_include_directories(hsms_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(hsms_tests
    tests/test_sanity.cpp
    tests/test_hsms.cpp
    tests/test_secs2.cpp
    tests/test_transport.cpp
    tests/test_robustness.cpp
)

target_link_libraries(hsms_tests
    PRIVATE
        hsms_core
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(hsms_tests)