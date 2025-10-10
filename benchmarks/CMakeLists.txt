# Core float parser benchmarks
add_executable(float_parser_benchmarks faster_parser/core/float_parser_benchmark.cpp)
target_link_libraries(float_parser_benchmarks
        PRIVATE
        faster_parser
        benchmark::benchmark
        benchmark::benchmark_main
)

add_custom_target(run_float_parser_benchmarks
        COMMAND $<TARGET_FILE:float_parser_benchmarks> --benchmark_format=console
        DEPENDS float_parser_benchmarks
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running float parser benchmarks..."
)

add_custom_target(run_float_parser_benchmarks_json
        COMMAND $<TARGET_FILE:float_parser_benchmarks> --benchmark_format=json --benchmark_out=float_parser_benchmark_results.json
        DEPENDS float_parser_benchmarks
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running float parser benchmarks with JSON output..."
)

# Binance Future Parser Benchmarks
add_executable(binance_future_benchmarks faster_parser/binance/future_benchmark.cpp)
target_link_libraries(binance_future_benchmarks
        PRIVATE
        faster_parser
        benchmark::benchmark
        benchmark::benchmark_main
)

add_custom_target(run_binance_future_benchmarks
        COMMAND $<TARGET_FILE:binance_future_benchmarks> --benchmark_format=console
        DEPENDS binance_future_benchmarks
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running Binance Future parser benchmarks..."
)

add_custom_target(run_binance_future_benchmarks_json
        COMMAND $<TARGET_FILE:binance_future_benchmarks> --benchmark_format=json --benchmark_out=binance_future_benchmark_results.json
        DEPENDS binance_future_benchmarks
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Running Binance Future parser benchmarks with JSON output..."
)