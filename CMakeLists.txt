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
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")

include(CheckCXXCompilerFlag)
include(CheckCXXSourceRuns)

function(test_cpu_feature feature_name test_code flags result_var)
    set(CMAKE_REQUIRED_FLAGS "${flags}")
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

if(CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    test_cpu_feature("AVX512F" "
        #include <immintrin.h>
        int main() {
            const char data[64] = {'0','1','2','3','4','5','6','7'};
            __m512i chunk = _mm512_loadu_si512(data);
            __m512i zero = _mm512_set1_epi8('0');
            __m512i digits = _mm512_sub_epi8(chunk, zero);
            __m128i low_128 = _mm512_extracti64x2_epi64(digits, 0);
            __mmask64 mask = _mm512_cmpge_epi8_mask(chunk, zero);
            return 0;
        }" "-mavx512f -mavx512bw -mavx512dq" CPU_SUPPORTS_AVX512F)

    if (CPU_SUPPORTS_AVX512F)
        list(APPEND SIMD_FLAGS "-mavx512f" "-mavx512bw" "-mavx512dq")
        list(APPEND SIMD_DEFINITIONS "HAS_AVX512")
        message(STATUS "AVX-512 support detected and enabled")
    endif()

    test_cpu_feature("AVX2" "
        #include <immintrin.h>
        int main() {
            const char data[32] = {'0','1','2','3','4','5','6','7'};
            __m256i chunk = _mm256_loadu_si256((__m256i*)data);
            __m256i zero = _mm256_set1_epi8('0');
            __m256i nine = _mm256_set1_epi8('9');
            __m256i ge_zero = _mm256_cmpgt_epi8(chunk, _mm256_sub_epi8(zero, _mm256_set1_epi8(1)));
            __m256i digits = _mm256_sub_epi8(chunk, zero);
            uint64_t raw = _mm256_extract_epi64(digits, 0);
            return 0;
        }" "-mavx2" CPU_SUPPORTS_AVX2)

    if (CPU_SUPPORTS_AVX2)
        list(APPEND SIMD_FLAGS "-mavx2")
        list(APPEND SIMD_DEFINITIONS "HAS_AVX2")
        message(STATUS "AVX2 support detected and enabled")
    endif()

    test_cpu_feature("SSE4.2" "
        #include <nmmintrin.h>
        int main() {
            const char data[16] = {'0','1','2','3','4','5','6','7'};
            __m128i chunk = _mm_loadu_si128((__m128i*)data);
            __m128i zero = _mm_set1_epi8('0');
            __m128i digits = _mm_sub_epi8(chunk, zero);
            __m128i ge_zero = _mm_cmpgt_epi8(chunk, _mm_subs_epi8(zero, _mm_set1_epi8(1)));
            uint64_t raw = _mm_extract_epi64(digits, 0);
            return 0;
        }" "-msse4.2" CPU_SUPPORTS_SSE42)

    if (CPU_SUPPORTS_SSE42)
        list(APPEND SIMD_FLAGS "-msse4.2")
        list(APPEND SIMD_DEFINITIONS "HAS_SSE42")
        message(STATUS "SSE4.2 support detected and enabled")
    endif()

elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "(arm)|(ARM)|(aarch64)|(AARCH64)")
    test_cpu_feature("NEON" "
        #include <arm_neon.h>
        int main() {
            const uint8_t data[16] = {'0','1','2','3','4','5','6','7','8','9','0','1','2','3','4','5'};
            uint8x16_t chunk = vld1q_u8(data);
            uint8x16_t zero = vdupq_n_u8('0');
            uint8x16_t nine = vdupq_n_u8('9');
            uint8x16_t ge_zero = vcgeq_u8(chunk, zero);
            uint8x16_t le_nine = vcleq_u8(chunk, nine);
            uint8x16_t valid = vandq_u8(ge_zero, le_nine);

            // Test parse_8_digits_simd functions
            const uint8_t digits_data[8] = {'1','2','3','4','5','6','7','8'};
            uint8x8_t digits_chunk = vld1_u8(digits_data);
            uint8x8_t digits_zero = vdup_n_u8('0');
            uint8x8_t digits = vsub_u8(digits_chunk, digits_zero);
            uint16x8_t digits_wide = vmovl_u8(digits);
            uint32x4_t digits_low = vmovl_u16(vget_low_u16(digits_wide));

            return 0;
        }" "" CPU_SUPPORTS_NEON)

    if (CPU_SUPPORTS_NEON)
        list(APPEND SIMD_DEFINITIONS "HAS_NEON")
        message(STATUS "NEON support detected and enabled")
    endif()
endif()

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
