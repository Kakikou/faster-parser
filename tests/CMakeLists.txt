# Core float parser tests
add_executable(float_parser_tests faster_parser/core/float_parser_tests.cpp)

target_link_libraries(float_parser_tests
        PRIVATE
        faster_parser
        gtest_main
        gmock_main
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(float_parser_tests PRIVATE -Wall -Wextra -Wpedantic)
endif ()

include(GoogleTest)
gtest_discover_tests(float_parser_tests)

# Binance Future Parser Tests
add_executable(binance_future_tests faster_parser/binance/future_tests.cpp)

target_link_libraries(binance_future_tests
        PRIVATE
        faster_parser
        gtest_main
        gmock_main
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(binance_future_tests PRIVATE -Wall -Wextra -Wpedantic)
endif ()

gtest_discover_tests(binance_future_tests)