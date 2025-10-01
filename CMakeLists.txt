cmake_minimum_required(VERSION 3.27)
project(faster-parser VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Désactiver tests et benchmarks quand utilisé via FetchContent
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    option(BUILD_TESTS "Build tests" ON)
    option(BUILD_BENCHMARKS "Build benchmarks" ON)
else()
    option(BUILD_TESTS "Build tests" OFF)
    option(BUILD_BENCHMARKS "Build benchmarks" OFF)
endif()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

option(USE_NATIVE_ARCH "Use -march=native for native CPU optimizations (should be deactivated during docker build)" ON)
if(USE_NATIVE_ARCH)
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
endif()

include(CheckCXXSourceRuns)

function(test_cpu_feature feature_name test_code flags result_var)
    set(CMAKE_REQUIRED_FLAGS "-O2 ${flags}")
    check_cxx_source_runs("${test_code}" ${result_var})
endfunction()

set(SIMD_FLAGS "")
set(SIMD_DEFINITIONS "")

add_library(faster_parser
        src/faster_parser/parsers.cpp
        src/faster_parser/parsers.h
)
target_include_directories(faster_parser PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(faster_parser PUBLIC cxx_std_23)

set_target_properties(faster_parser PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION 1
)

add_library(faster_parser::faster_parser ALIAS faster_parser)

# Enable specific instruction sets based on architecture
if (CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        include(CheckCXXCompilerFlag)

        # Test SSE4.2 support
        test_cpu_feature("SSE4.2" "
            #include <nmmintrin.h>
            int main() {
                __m128i a = _mm_set1_epi32(1);
                __m128i b = _mm_set1_epi32(2);
                __m128i result = _mm_cmpgt_epi32(a, b);
                return 0;
            }" "-msse4.2" CPU_SUPPORTS_SSE42)

        if (CPU_SUPPORTS_SSE42)
            list(APPEND SIMD_FLAGS "-msse4.2")
            list(APPEND SIMD_DEFINITIONS "HAS_SSE42")
            message(STATUS "SSE4.2 support detected and enabled")
        else()
            message(STATUS "SSE4.2 not supported on this CPU")
        endif()

        # Test AVX2 support
        test_cpu_feature("AVX2" "
            #include <immintrin.h>
            int main() {
                __m256i a = _mm256_set1_epi32(1);
                __m256i b = _mm256_set1_epi32(2);
                __m256i result = _mm256_cmpgt_epi32(a, b);
                return 0;
            }" "-mavx2" CPU_SUPPORTS_AVX2)

        if (CPU_SUPPORTS_AVX2)
            list(APPEND SIMD_FLAGS "-mavx2")
            list(APPEND SIMD_DEFINITIONS "HAS_AVX2")
            message(STATUS "AVX2 support detected and enabled")
        else()
            message(STATUS "AVX2 not supported on this CPU")
        endif()

        # Test AVX-512 support
        test_cpu_feature("AVX512F" "
            #include <immintrin.h>
            int main() {
                __m512i a = _mm512_set1_epi32(1);
                __m512i b = _mm512_set1_epi32(2);
                __mmask16 mask = _mm512_cmpgt_epi32_mask(a, b);
                return 0;
            }" "-mavx512f" CPU_SUPPORTS_AVX512F)

        if (CPU_SUPPORTS_AVX512F)
            list(APPEND SIMD_FLAGS "-mavx512f")
            list(APPEND SIMD_DEFINITIONS "HAS_AVX512")
            message(STATUS "AVX512F support detected and enabled")
        else()
            message(STATUS "AVX512F not supported on this CPU")
        endif()

    endif ()
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64|ARM64")
    # ARM64/Apple Silicon optimizations
    # Test NEON support
    test_cpu_feature("NEON" "
        #include <arm_neon.h>
        int main() {
            uint8x16_t a = vdupq_n_u8(1);
            uint8x16_t b = vdupq_n_u8(2);
            uint8x16_t result = vcgtq_u8(a, b);
            return 0;
        }" "" CPU_SUPPORTS_NEON)

    if (CPU_SUPPORTS_NEON)
        list(APPEND SIMD_DEFINITIONS "HAS_NEON")
        message(STATUS "NEON support detected and enabled")
    else()
        message(STATUS "NEON not supported on this CPU")
    endif()
endif ()

# Apply SIMD flags and definitions to the target
if (SIMD_FLAGS)
    target_compile_options(faster_parser PRIVATE ${SIMD_FLAGS})
endif()

if (SIMD_DEFINITIONS)
    target_compile_definitions(faster_parser PRIVATE ${SIMD_DEFINITIONS})
endif()

include(cmake/Dependencies.cmake)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif ()

if (BUILD_BENCHMARKS)
    add_subdirectory(benchmarks)
endif ()

include(GNUInstallDirs)

install(FILES src/faster_parser/parsers.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        COMPONENT Development)

install(TARGETS faster_parser
        EXPORT faster_parser-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT faster_parser-targets
        FILE faster_parser-targets.cmake
        NAMESPACE faster_parser::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/faster_parser)
