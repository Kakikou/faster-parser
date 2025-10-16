# Faster Parser

A high-performance C++ library for floating-point number parsing and specialized JSON parsers, optimized with SIMD instructions (SSE4.2, AVX2, AVX-512, NEON).

## Features

- **Ultra-fast** : SIMD optimizations for different architectures (x86_64, ARM64)
- **Float Parser** : Optimized for financial prices (8 fixed decimals)
- **Binance Parser** : High-performance parser for Binance WebSocket messages
- **Compiled library** : Single compilation, fast linking
- **Benchmarked** : Performance comparisons with GoogleBenchmark
- **CMake** : Modern configuration with installation and export

## Supported Architectures

- **x86_64** : SSE4.2, AVX2, AVX-512
- **ARM64** : NEON (Apple Silicon, Linux ARM64)
- **Fallback** : Scalar implementation for other architectures

## Installation

### Prerequisites

- CMake 3.27+
- C++23 compatible compiler (GCC, Clang, MSVC)

### Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### System Installation

```bash
sudo make install
```

### Build Options

| Option                  | Description                                 | Default |
|-------------------------|---------------------------------------------|---------|
| `BUILD_TESTS`           | Build tests (requires GoogleTest)           | ON      |
| `BUILD_BENCHMARKS`      | Build benchmarks (requires GoogleBenchmark) | ON      |
| `BUILD_MAIN_EXECUTABLE` | Build main executable                       | ON      |

Example:
```bash
cmake .. -DBUILD_TESTS=OFF -DBUILD_BENCHMARKS=OFF
```

## Usage

### Basic Usage

```cpp
#include "faster_parser/parsers.h"
using namespace core::fast_float_parser;

// High-performance parsing with SIMD optimizations
double price = parse_float("25.35190000");
double negative = parse_float("-123.45678900");
double integer = parse_float("12345");
double scientific = parse_float("1.23e-4"); // fallback to standard parsing
```

### CMake Integration

There are several ways to integrate faster-parser into your CMake project:

#### Method 1: FetchContent (Recommended)

Add this to your `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.27)
project(your_project)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    faster_parser
    GIT_REPOSITORY https://github.com/Kakikou/faster-parser.git
    GIT_TAG main  # or specific version like v1.0.0
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(faster_parser)

add_executable(your_app main.cpp)
target_link_libraries(your_app PRIVATE faster_parser::faster_parser)
```

#### Method 2: Git Submodule

```bash
# Add as git submodule
git submodule add https://github.com/your-username/faster-parser.git third-party/faster-parser
git submodule update --init --recursive
```

Then in your `CMakeLists.txt`:
```cmake
# Add subdirectory
add_subdirectory(third-party/faster-parser)

# Link with your target
target_link_libraries(your_app PRIVATE faster_parser::faster_parser)
```

**Note**: When using `FetchContent` or as a subdirectory, tests and benchmarks are automatically disabled to speed up compilation.

#### Method 3: System Installation

```bash
# Install system-wide
git clone https://github.com/your-username/faster-parser.git
cd faster-parser
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
sudo make install
```

Then in your `CMakeLists.txt`:
```cmake
find_package(faster_parser REQUIRED)
target_link_libraries(your_app PRIVATE faster_parser::faster_parser)
```

## API

### Float Parser

```cpp
namespace core::fast_float_parser {
    /**
     * @brief High-performance floating-point parser with SIMD optimizations
     * @param str String view containing the number to parse
     * @return Parsed floating-point number
     */
    double parse_float(std::string_view str);
}
```

#### Simple and Clean

The library exposes a single, powerful function that handles all parsing scenarios:
- ✅ **Integers**: `parse_float("12345")`
- ✅ **Decimals**: `parse_float("123.456")`
- ✅ **Negative numbers**: `parse_float("-123.456")`
- ✅ **Financial precision**: `parse_float("25.35190000")`
- ✅ **Scientific notation**: Falls back to `std::strtod` automatically
- ✅ **SIMD optimized**: Automatically uses best available instruction set


#### Performance Comparison:

| Platform            | SIMD | `parse_float` | `strtod` | `std::stod` | Speedup vs strtod | Speedup vs std::stod |
|---------------------|------|---------------|----------|-------------|-------------------|----------------------|
| Apple M1 Pro        | NEON | 5.84 ns       | 18.4 ns  | 30.4 ns     | **3.15×**         | **5.21×**            |
| Raspberry Pi 5      | NEON | 21.8 ns       | 106 ns   | 107 ns      | **4.86×**         | **4.91×**            |
| Intel Ultra 7 265** | AVX2 | 9.28 ns       | 26.6 ns  | 26.5 ns     | **2,86×**         | **2,86×**            |

\* _Tested with HT off, P-Cores isolated, CPU Governor at Performance, taskset on P-Core_\
\** _P-Cores isolated, CPU Governor at Performance, taskset on P-Core_

### Binance Parser

High-performance parser for Binance WebSocket messages with SIMD optimizations (AVX-512, AVX2, NEON).

**Architecture Selection**: The library automatically selects the best available SIMD implementation at compile time:
- **AVX-512** (highest priority): 64-byte parallel processing on modern Intel/AMD CPUs
- **AVX2**: 32-byte parallel processing
- **NEON**: ARM64 optimizations for Apple Silicon and ARM processors
- **Scalar**: Portable fallback for all other architectures

```cpp
#include "faster_parser/binance/future.h"
using namespace core::binance::future;

// Parse Binance book ticker message
std::string_view json_message = R"({"u":12345,"s":"BTCUSDT","b":"50000.12","B":"1.5","a":"50001.34","A":"2.3","T":1234567890,"E":1234567890})";
BookTicker ticker = parse_book_ticker(json_message);

// Access parsed fields
std::cout << "Symbol: " << ticker.symbol << "\n";
std::cout << "Best bid: " << ticker.best_bid_price << "\n";
std::cout << "Best ask: " << ticker.best_ask_price << "\n";
```

#### Supported Messages

- ✅ **Book Ticker** (`@bookTicker`): Real-time best bid/ask prices
- ✅ **Aggregated Trade** (`@aggTrade`): Aggregated trade pushed for fills with same prices
- 🔄 **Additional message types coming soon**

#### Performance Comparison

Comparison between faster-parser and simdjson for parsing Binance messages:

| Platform            | SIMD | faster-parser | simdjson | Speedup vs simdjson |
|---------------------|------|---------------|----------|---------------------|
| Apple M1 Pro        | NEON | 56.8 ns       | 192 ns   | **3.38×**           |
| Raspberry Pi 5      | NEON | TBD           | TBD      | **TBD**             |
| Intel Ultra 7 265** | AVX2 | 78.6 ns       | 152 ns   | **1.93×**           |

\* _Tested with HT off, P-Cores isolated, CPU Governor at Performance, taskset on P-Core_\
\** _P-Cores isolated, CPU Governor at Performance, taskset on P-Core_

**Benchmark**: `bm_faster_parser_mixed_workload` - Parses complete book ticker JSON messages including symbol extraction, price/volume parsing

## Tests

### Automatic Dependencies

Dependencies are automatically downloaded and built from source using CMake FetchContent:
- **GoogleTest v1.17.0** - Testing framework

No manual installation required! Just build and the dependencies will be fetched automatically.

#### Dependency Architecture

The project uses a centralized dependency management system:

1. **`cmake/Dependencies.cmake`** - Single source of truth for all dependencies
2. **Conditional fetching** - Only downloads what's needed based on build options
3. **Version pinning** - Uses specific tags for reproducible builds
4. **Shallow clones** - Faster downloads with `GIT_SHALLOW ON`
5. **Offline support** - Uses `FETCHCONTENT_UPDATES_DISCONNECTED`

This approach provides:
- ✅ **Reproducible builds** across all platforms
- ✅ **Fast CI/CD** with minimal network overhead
- ✅ **Easy maintenance** with centralized version management
- ✅ **No version conflicts** between dependencies

### Running Tests

```bash
cd build
ctest --output-on-failure
# or directly:
./tests/parser_tests
```

## Benchmarks

### Automatic Dependencies

Benchmark dependencies are automatically downloaded and built from source:
- **Google Benchmark v1.9.4** - Performance measurement framework

### Running

```bash
cd build
make run_benchmarks          # Console output
make run_benchmarks_json     # JSON output
# or directly:
./benchmarks/parser_benchmarks
```

## Project Structure

```
faster-parser/
├── src/
│   └── faster_parser/
│       ├── parsers.h                      # Header declarations
│       └── parsers.cpp                    # Implementation with SIMD optimizations
├── tests/
│   ├── CMakeLists.txt                     # Test configuration
│   └── fast_float_parser_tests.cpp       # Test implementation
├── benchmarks/
│   ├── CMakeLists.txt                     # Benchmark configuration
│   └── float_parser_benchmark.cpp        # Benchmark implementation
├── cmake/
│   └── Dependencies.cmake                 # Centralized dependency management
├── example/
│   ├── CMakeLists.txt                     # Usage example
│   └── example.cpp                        # Example implementation
├── CMakeLists.txt                         # Main configuration
├── .gitignore                             # Git ignore rules
└── README.md                              # This file
```

### Key Features

- **Centralized Dependencies**: All FetchContent managed in `cmake/Dependencies.cmake`
- **Modular Structure**: Clear separation between header, implementation, tests, and benchmarks
- **Zero-Config**: No manual dependency installation required
- **Single Compilation**: SIMD code compiled once, fast linking, no template instantiation overhead
- **Comprehensive Testing**: Full test suite with GoogleTest
- **Performance Benchmarks**: Detailed benchmarks with Google Benchmark

## Optimizations

### SIMD

- **AVX-512**: 64-byte parallel processing
- **AVX2**: 32-byte parallel processing
- **SSE4.2**: 16-byte parallel processing
- **NEON**: ARM64 optimizations with `vminvq_u8`

### Algorithms

- **Batch processing**: Processing in 8-digit blocks
- **Branch prediction**: Minimizing conditional branches
- **Cache efficiency**: Precalculated tables and optimized memory access
- **Unrolled loops**: Critical loop unrolling

## Performance

This library is particularly efficient for:

- ✅ Financial prices (8-decimal format)
- ✅ Trading volumes
- ✅ High-frequency data
- ✅ Real-time processing
- ✅ Large CSV/JSON file parsing

## Status & Production Use

⚠️ **Development Status**: This library is not fully tested across all edge cases and platforms. However, it has been successfully deployed and tested in production environments, specifically in trading bots handling high-frequency financial data parsing.

**Production Usage**:
- ✅ Used in live trading bots for real-time price parsing
- ✅ Handles millions of financial transactions daily
- ✅ Proven stability in high-frequency trading environments
- ✅ Optimized for financial price formats (8 decimal precision)

While comprehensive test coverage is still in development, the library has demonstrated reliability in demanding production scenarios. Use at your own discretion and test thoroughly in your specific use case.

## Limitations

- Optimized for numbers < 10^18 (double precision limit)
- No native support for scientific notation (e/E)
- Optimal performance on SIMD architectures

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Contributors

- Kevin Rodrigues - Main author