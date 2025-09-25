add_executable(parser_benchmarks float_parser_benchmark.cpp)
target_link_libraries(parser_benchmarks
        PRIVATE
        faster_parser
        benchmark::benchmark
        benchmark::benchmark_main
)

add_custom_target(run_benchmarks
        COMMAND $<TARGET_FILE:parser_benchmarks> --benchmark_format=console
        DEPENDS parser_benchmarks
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running parser benchmarks..."
)

add_custom_target(run_benchmarks_json
        COMMAND $<TARGET_FILE:parser_benchmarks> --benchmark_format=json --benchmark_out=benchmark_results.json
        DEPENDS parser_benchmarks
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running parser benchmarks with JSON output..."
)