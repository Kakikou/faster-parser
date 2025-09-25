add_executable(parser_tests fast_float_parser_tests.cpp)

target_link_libraries(parser_tests
        PRIVATE
        faster_parser
        gtest_main
        gmock_main
)

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(parser_tests PRIVATE -Wall -Wextra -Wpedantic)
endif ()

include(GoogleTest)
gtest_discover_tests(parser_tests)