cmake_minimum_required(VERSION 3.15)
project(SECS_HSMS_Emulator VERSION 1.0.0 LANGUAGES CXX)

# Force modern C++17 standard across all compilers
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json for IDE language server support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enable strict compiler warnings to catch errors early
if(MSVC)
    add_compile_options(/W4 /WX) # W4 = High warnings, WX = Treat warnings as errors
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Define core static library target
add_library(hsms_core
    src/common.cpp
	src/byte_order.cpp
	src/header.cpp
	src/state_machine.cpp
)

# Specify header search paths for the library target
target_include_directories(hsms_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Automatically download and integrate GoogleTest framework
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz
)
# Force MSVC runtime sharing to prevent linking mismatches on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable CMake testing infrastructure
enable_testing()

# Define the unit test executable target
add_executable(hsms_tests
    tests/test_sanity.cpp
	tests/test_hsms.cpp
)

# Link the unit test target against core library and GoogleTest
target_link_libraries(hsms_tests
    PRIVATE
        hsms_core
        GTest::gtest_main
)

# Automatically register discovered tests with CTest runner
include(GoogleTest)
gtest_discover_tests(hsms_tests)